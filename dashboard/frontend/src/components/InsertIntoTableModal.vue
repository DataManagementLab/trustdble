<template>
  <div
    class="modal-card"
    style="width: auto"
  >
    <header class="modal-card-head">
      <p class="modal-card-title">
        Insert into {{ tableName }}
      </p>
      <button
        type="button"
        class="delete"
        @click="$emit('on-cancel')"
      />
    </header>

    <section class="modal-card-body">
      <b-field
        v-for="num in num_rows"
        :key="num"
        grouped
      >
        <b-field
          v-for="(column, columnIndex) in columns[num-1]"
          :key="columnIndex"
          :label="num-1 === 0 ? column.name : ''"
          grouped
        >
          <b-input
            v-model="tuples[num-1][column.name]"
            :type="column.type === 'date' ? 'date' : null"
          />
        </b-field>
      </b-field>
      <div class="buttons">
        <b-button
          style="color: #000000B3"
          label="Add Row"
          type="is-link  is-light"
          @click="addRow()"
        />
        <b-button
          v-if="num_rows > 1"
          label="Remove Row"
          type="is-danger is-light"
          @click="removeRow()"
        />
      </div>
    </section>

    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('on-cancel')"
      />
      <b-button
        label="Insert"
        type="is-primary submit"
        @click="insertIntoTable()"
      />
    </footer>
    <b-loading
      v-model="isLoading"
      :can-cancel="false"
    />
  </div>
</template>

<script>
import axios from 'axios'

export default {
  name: 'InsertIntoTableModal',
  props: {
    dbName: {
      type: String,
      default: null
    },
    tableName: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      num_rows: 1,
      columns: [],
      obj: null,
      tuples: [],
      isLoading: false
    }
  },
  async mounted () {
    this.obj = await this.fetchTableMetadata()
    this.columns.push(this.obj)
    this.tuples.push(Object.fromEntries(this.obj.map(column => [column.name, ''])))
  },
  methods: {
    showLog (val) {
    },
    async addRow () {
      this.columns.push(this.obj)
      this.tuples.push(Object.fromEntries(this.obj.map(column => [column.name, ''])))
      this.num_rows = this.num_rows + 1
    },
    removeRow () {
      if (this.num_rows > 0) {
        this.num_rows = this.num_rows - 1
        this.columns.pop()
        this.tuples.pop()
      }
    },

    async fetchTableMetadata () {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}`)
        return response.data.columns
      } catch (error) {
        this.showErrorPrompt(`Error: ${error.message}`)
      }
    },
    async insertIntoTable () {
      if (!this.validateInput()) {
        this.showInvalidInputErrorPrompt('Please fill out all fields')
        return
      }
      this.isLoading = true
      const tuples = { tuples: this.tuples }
      try {
        await axios.put(`/api/databases/${this.dbName}/tables/${this.tableName}/tuples`, tuples)
        this.messagePrompt({ message: 'Row inserted', type: 'is-success' })
        this.$emit('close')
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    },
    validateInput () {
      let isValid = true
      this.tuples.every(tuple => Object.entries(tuple).map(entry => {
        if (entry[1] === '') {
          isValid = false
        }
      }))
      return isValid
    }
  }
}
</script>
