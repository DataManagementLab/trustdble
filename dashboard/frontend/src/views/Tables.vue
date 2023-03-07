<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar>
      <router-link :to="{name: 'databases'}">
        <b-icon
          icon="chevron-left"
          custom-size="default"
        />
      </router-link>
      Table: {{ tableName }}
    </HeroBar>
    <section class="section is-main-section">
      <!-- If shared table -->
      <div
        v-if="isSharedTable && tableStates.length"
        class="columns mb-0 is-vcentered"
      >
        <div class="column is-4">
          <span>State: </span>
          <span v-if="!isUpdatingTableStates">
            <span v-if="!currentTableState || tableStateIndex === tableStates.length-1">
              Latest
            </span>
            <span v-if="currentTableState && tableStateIndex !== tableStates.length-1">
              {{ tableStateIndex + 1 }}
            </span>
          </span>
          <b-icon
            v-if="isUpdatingTableStates"
            class="spin-icon"
            icon="loading"
            size="is-small"
          />
        </div>

        <div class="column is-8">
          <div class="columns is-mobile">
            <div class="column is-4">
              <b-field>
                <b-button
                  class="is-light"
                  icon-left="arrow-left"
                  :disabled="isFirstTableState() || isUpdatingTableStates"
                  @click="previousTableState()"
                >
                  Previous state
                </b-button>
                <b-button
                  class="is-light"
                  icon-right="arrow-right"
                  :disabled="isLastTableState() || isUpdatingTableStates"
                  @click="nextTableState()"
                >
                  Next state
                </b-button>
              </b-field>
            </div>

            <div class="column is-8">
              <b-field
                grouped
                position="is-right"
              >
                <b-tooltip
                  label="Reload"
                  type="is-lighter"
                >
                  <b-button
                    class="is-lighter"
                    icon-right="refresh"
                    @click="fetchData()"
                  />
                </b-tooltip>
              </b-field>
            </div>
          </div>
        </div>
      </div>

      <!-- If local table  -->
      <div
        v-if="!isSharedTable || !tableStates.length"
        class="columns mb-0"
      >
        <div class="column is-12 mb-0">
          <b-field
            grouped
            position="is-right"
          >
            <b-tooltip
              label="Reload"
              type="is-lighter"
            >
              <b-button
                class="is-lighter"
                icon-right="refresh"
                @click="fetchData()"
              />
            </b-tooltip>
          </b-field>
        </div>
      </div>

      <CardComponent
        class="has-table has-mobile-sort-spaced"
        :title="tableName"
        :icon="isSharedTable ? 'table-eye' : 'table'"
      >
        <GenericTable
          :key="tableComponentKey"
          :db-name="dbName"
          :table-name="tableName"
          :tuples="tuples"
          :columns="columns"
          :is-shared-table="isSharedTable"
          :is-latest-state="isLastTableState()"
          :is-history-exist="tableStates.length"
          @fetch="fetchData(false)"
        />
      </CardComponent>

      <b-button
        class="button is-link"
        @click="isInsertModalActive = true"
      >
        Insert
      </b-button>

      <b-modal
        v-model="isInsertModalActive"
        has-modal-card
        trap-focus
        :destroy-on-hide="true"
        aria-role="dialog"
        aria-label="Invite Upload Modal"
        aria-modal
      >
        <template v-if="isInsertModalActive">
          <InsertIntoTableModal
            :db-name="dbName"
            :table-name="tableName"
            @on-cancel="isInsertModalActive = false"
            @close="isInsertModalActive = false, fetchData(true)"
          />
        </template>
      </b-modal>
    </section>
  </div>
</template>

<script>
import CardComponent from '@/components/CardComponent'
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'
import GenericTable from '../components/GenericTable.vue'
import InsertIntoTableModal from '../components/InsertIntoTableModal.vue'
import axios from 'axios'

export default {
  name: 'Tables',
  components: { HeroBar, TitleBar, CardComponent, GenericTable, InsertIntoTableModal },
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
      HISTORY_TABLE_PREFIX: 'history_',
      tableComponentKey: 0,
      columns: [],
      tuples: [],
      isSharedTable: null,
      tableStates: [],
      tableStateIndex: null,
      currentTableState: null,
      isInsertModalActive: false,
      isUpdatingTableStates: false
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE',
        this.dbName
      ]
    }
  },
  created () {
    this.fetchData(false)
  },
  methods: {
    async fetchData (isUpdate) {
      const tableData = await this.fetchLiveTable()
      if (tableData) {
        this.isSharedTable = tableData.isShared
        this.columns = tableData.columns
        this.forceRerenderOfTableComponent()
        if (tableData.isShared) {
          await this.fetchTableStates()
        }
      }
      if (this.tableStates.length) {
        this.tuples = await this.fetchTableAtTableState(this.currentTableState)
      } else if (tableData) {
        this.tuples = tableData.tuples
      }
      if (isUpdate) {
        this.updateTableStates()
      }
    },
    async fetchLiveTable () {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}/tuples`)
        return response.data
      } catch (error) {
        this.showErrorPrompt(error)
        return null
      }
    },
    async updateTableStates () {
      this.isUpdatingTableStates = true
      await this.sleep(1000)
      await this.fetchTableStates()
      this.isUpdatingTableStates = false
      await this.fetchData()
    },
    async fetchTableStates () {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}/history/states`)
        this.tableStates = response.data.states
        this.tableStateIndex = this.tableStates.length - 1
        this.setCurrentTableState(this.tableStates[this.tableStates.length - 1])
      } catch (error) {
        this.tableStates = []
      }
    },
    async fetchTableAtTableState (tableState) {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}/history/states/${tableState}`)
        if (this.columns[0].name !== 'primaryKeyHash') this.columns = [{ name: 'primaryKeyHash' }].concat(this.columns)
        return response.data.tuples
          .filter((tuple) => tuple.transactionType === 'PUT')
          .map(state => Object.fromEntries([state.primaryKeyHash].concat(state.dataValue.split(',')).map((val, index) => [this.columns[index].name, val])))
      } catch (error) {
        return []
      }
    },
    setCurrentTableState (tableState) {
      this.currentTableState = tableState
    },
    async nextTableState () {
      this.tableStateIndex += 1
      this.setCurrentTableState(this.tableStates[this.tableStateIndex])
      this.tuples = await this.fetchTableAtTableState(this.currentTableState)
      this.forceRerenderOfTableComponent()
    },
    async previousTableState () {
      this.tableStateIndex -= 1
      this.setCurrentTableState(this.tableStates[this.tableStateIndex])
      this.tuples = await this.fetchTableAtTableState(this.currentTableState)
      this.forceRerenderOfTableComponent()
    },
    isLastTableState () {
      return (this.currentTableState === this.tableStates[this.tableStates.length - 1] || this.tableStates.length === 0)
    },
    isFirstTableState () {
      return (this.currentTableState === this.tableStates[0] || this.tableStates.length === 0)
    },
    forceRerenderOfTableComponent () {
      this.tableComponentKey += 1
    },
    sleep (ms) {
      return new Promise((resolve) => {
        setTimeout(resolve, ms)
      })
    }
  }
}
</script>

<style scoped>
  .spin-icon {
    animation-name: spin;
    animation-duration: 500ms;
    animation-iteration-count: infinite;
    animation-timing-function: linear;
  }
  @keyframes spin {
    from {
        transform:rotate(0deg);
    }
    to {
        transform:rotate(360deg);
    }
  }
</style>
