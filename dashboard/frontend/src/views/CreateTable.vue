<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar :has-right-visible="false">
      <router-link :to="'databases'">
        <b-icon
          icon="chevron-left"
          custom-size="default"
        />
      </router-link>
      Create Table
    </HeroBar>
    <section class="section is-main-section">
      <CardComponent>
        <b-field grouped>
          <b-field label="Table Name">
            <b-input
              v-model="tableName"
              placeholder="Table Name"
            />
          </b-field>
          <b-field
            v-if="dbType !== 'LOCAL'"
            label="Shared Table"
          >
            <b-checkbox
              v-model="isSharedTable"
              placeholder="Shared"
            />
          </b-field>
        </b-field>

        <b-field
          v-for="(column, counter) in columns"
          :key="counter"
          grouped
        >
          <b-field :label="column.isFirstLine ? 'Column Name' : ''">
            <b-input
              v-model="column.name"
              placeholder="Column Name"
            />
          </b-field>

          <b-field :label="column.isFirstLine ? 'Type' : ''">
            <b-select v-model="column.type">
              <option
                v-for="(option, index) in dataTypes"
                :key="index"
              >
                {{ option }}
              </option>
            </b-select>
          </b-field>

          <b-field :label="column.isFirstLine ? 'Length' : ''">
            <b-input
              v-model="column.length"
              type="number"
              :disabled="!isLengthType(column.type)"
              controls-alignment="right"
              placeholder="Length"
            />
          </b-field>

          <b-field :label="column.isFirstLine ? 'PK' : ''">
            <b-checkbox v-model="column.isPrimaryKey" />
          </b-field>

          <b-field :label="column.isFirstLine ? 'NN' : ''">
            <b-checkbox v-model="column.isNotNull" />
          </b-field>
        </b-field>

        <div class="buttons">
          <b-button
            style="color: #000000B3"
            label="Add Column"
            type="is-link  is-light"
            @click="addColumn()"
          />
          <b-button
            label="Remove Column"
            type="is-danger is-light"
            @click="removeColumn()"
          />
        </div>

        <b-loading
          v-model="isLoading"
          :can-cancel="false"
        />
        <b-button
          label="Create Table"
          type="is-primary submit"
          @click="createTable()"
        />
      </CardComponent>
    </section>
  </div>
</template>
<script>
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'
import axios from 'axios'
import CardComponent from '../components/CardComponent.vue'
export default {
  name: 'CreateTableModal',
  components: { HeroBar, TitleBar, CardComponent },
  props: {
    dbName: {
      type: String,
      required: true
    },
    dbType: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      dataTypes: ['TINYINT', 'SMALLINT', 'MEDIUMINT', 'INT', 'BIGINT', 'CHAR', 'VARCHAR', 'DATE'],
      lengthTypes: ['VARCHAR', 'CHAR'],
      tableName: '',
      isSharedTable: this.dbType !== 'LOCAL',
      columns: [
        {
          name: '',
          type: '',
          length: null,
          isPrimaryKey: false,
          isNotNull: false,
          isFirstLine: true
        }
      ],
      isLoading: false
    }
  },
  computed: {
    titleStack () {
      return [
        'TrustDBle',
        this.dbName
      ]
    }
  },
  methods: {
    addColumn () {
      this.columns.push({
        name: '',
        type: '',
        length: null,
        isPrimaryKey: false,
        isNotNull: false,
        isFirstLine: false
      })
    },
    removeColumn () {
      if (this.columns.length <= 1) {
        return
      }
      this.columns.pop()
    },
    async createTable () {
      if (!this.validateInput()) {
        this.showInvalidInputErrorPrompt('Please fill out all fields')
        return
      }
      this.isLoading = true
      const table = this.transformInput()
      try {
        await axios.post(`api/databases/${this.dbName}/tables`, table)
        this.messagePrompt({ message: `Table ${this.tableName} created`, type: 'is-success' })
        this.$router.go(-1)
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    },
    validateInput () {
      let isValid = true
      this.columns.forEach(column => {
        if (!this.tableName || !column.name || !column.type) {
          isValid = false
        }
        if (this.isLengthType(column.type) && !column.length) {
          isValid = false
        }
      })
      return isValid
    },
    transformInput () {
      const arrayOfColumns = []
      this.columns.forEach(column => {
        arrayOfColumns.push({
          name: column.name,
          type: this.isLengthType(column.type) ? column.type.concat('(', column.length, ')') : column.type,
          isPrimaryKey: column.isPrimaryKey,
          isNotNull: column.isNotNull
        })
      })
      return { tableName: this.tableName, isSharedTable: this.isSharedTable, columns: arrayOfColumns }
    },
    isLengthType (dataType) {
      let isLengthType = false
      this.lengthTypes.forEach(lengthType => {
        if (lengthType === dataType) {
          isLengthType = true
        }
      })
      return isLengthType
    }
  }
}
</script>
