<template>
  <div class="sidebar-page">
    <section class="Header">
      <b-sidebar
        v-model="open"
        type="is-light"
        :fullheight="true"
        :fullwidth="false"
        :overlay="false"
        :right="true"
      >
        <div class="collapse-parent">
          <h1 style="textAlign:center; margin:10px 0px 10px 0px">
            Databases
          </h1>
          <b-collapse
            v-for="(database, index) of databases"
            :key="index"
            class="card"
            animation="slide"
            :open="isOpen == index"
            :aria-id="'contentIdForA11y5-' + index"
            @open="isOpen = index; query = `USE ${database.name}`; runQuery()"
          >
            <template #trigger="props">
              <div
                class="card-header"
                role="button"
                :aria-controls="'contentIdForA11y5-' + index"
                :aria-expanded="props.open"
              >
                <p class="card-header-title">
                  {{ database.name }}
                </p>
                <a class="card-header-icon">
                  <b-icon
                    :icon="props.open ? 'menu-down' : 'menu-up'"
                  />
                </a>
              </div>
            </template>
            <div class="card-content">
              Tables:
              <div class="modified-content">
                <ul>
                  <li
                    v-for="(table, table_index) in database.tables"
                    :key="table_index"
                  >
                    <a
                      :key="table_index"
                      @click="query = `select * from ${database.name}.${table.name};`; runQuery()"
                    >
                      {{ table.name }}
                    </a>
                  </li>
                </ul>
              </div>
            </div>
          </b-collapse>
          <!--  -->
        </div>
      </b-sidebar>
      <div class="FlexBox">
        <div>
          <b-icon
            icon="database-search"
            size="is-small"
          />
          SQL Workbench
        </div>
        <div>
          <b-button
            type="is-success"
            icon-left="play"
            @click="runQuery"
          >
            RUN
          </b-button>
        </div>
      </div>
      <div class="WidthControl">
        <div class="Query">
          <b-input
            v-model="query"
            maxlength="1000"
            type="textarea"
          />
        </div>
        <div class="Result">
          <b-icon
            icon="grid"
            size="is-small"
          />
          Output
          <b-message
            v-if="isError"
            title="Error while running query"
            type="is-danger"
            aria-close-label="Close message"
          >
            {{ errorMessage }}
          </b-message>
          <Tabs
            v-else
            :base-tabs="baseTabs"
          />
        </div>
      </div>
      <b-loading
        v-model="isLoading"
        :can-cancel="false"
      />
    </section>
  </div>
</template>

<script>
import Tabs from '@/components/Tabs'
import axios from 'axios'

export default {
  name: 'SQLWorkbench',
  components: {
    Tabs
  },
  data () {
    return {
      query: '',
      isLoading: false,
      num_of_queries: 0,
      combined_result: [],
      baseTabs: [],
      isError: false,
      open: true,
      errorMessage: '',
      HISTORY_TABLE_PREFIX: 'history_',
      SYSTEM_TABLES: ['meta_table', 'data_chains', 'shared_tables', 'key_store'],
      SYSTEM_DATABASES: ['information_schema', 'mysql', 'performance_schema', 'sys', 'trustdble'],
      LOCAL_DB_TYPE: 'LOCAL',
      databases: [],
      isOpen: -1
    }
  },
  computed: {
    tabs () {
      const baseTabs = []
      if (this.num_of_queries > 1) {
        if (this.combined_result.data) {
          this.combined_result.data.forEach((res, index) => {
            baseTabs.push(
              {
                id: `query ${index + 1}`,
                label: `query ${index + 1}`,
                content: res
              }
            )
          })
        } else {
          baseTabs.push({
            id: 'Error',
            label: 'Error',
            content: 'An error Occurred'
          })
        }
      } else {
        baseTabs.push(
          {
            id: `query ${1}`,
            label: `query ${1}`,
            content: this.combined_result.data
          }
        )
      }
      return baseTabs
    }
  },
  created () {
    this.fetchData()
  },
  methods: {
    async fetchData () {
      this.isLoading = true
      let databases = await this.fetchDatabases()
      databases = databases.filter(db => !this.SYSTEM_DATABASES.includes(db.name))
      for (const db of databases) {
        const { tables, unloadedTables } = await this.fetchTables(db.name)
        db.tables = tables.filter(table => !table.name.startsWith(this.HISTORY_TABLE_PREFIX) && !this.SYSTEM_TABLES.includes(table.name))
        db.unloadedTables = unloadedTables
      }
      this.databases = databases
      this.isLoading = false
    },
    async fetchDatabases () {
      try {
        const response = await axios.get('api/databases')
        return response.data.databases
      } catch (error) {
        this.showErrorPrompt(error)
        return []
      }
    },
    async fetchTables (dbName) {
      try {
        const response = await axios.get(`api/databases/${dbName}/tables`)
        return { tables: response.data.tables, unloadedTables: response.data.unloaded_tables }
      } catch (error) {
        this.showErrorPrompt(error)
      }
    },
    async runQuery () {
      this.isLoading = true
      this.combined_result = []
      this.isError = false
      {
        const body = { sqlCommand: this.query }
        this.num_of_queries = this.query.trim().split(';').filter((item) => item !== '').length
        try {
          this.combined_result = await axios.post('api/sql', body)
        } catch (error) {
          this.combined_result = []
          this.isError = true
          this.errorMessage = `${error.response.data.code}: ${error.response.data.message}`
        } finally {
          this.isLoading = false
          this.baseTabs = this.tabs
        }
      }
    }

  }
}

</script>
