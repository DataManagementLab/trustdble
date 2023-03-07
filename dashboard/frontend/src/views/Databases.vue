<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar>
      Databases
      <div
        slot="right"
        class="buttons"
      >
        <b-button
          type="is-primary"
          icon-left="plus"
          @click="isCreateDatabaseModalActive = true"
        >
          Create Database
        </b-button>
        <b-button
          type="is-primary"
          icon-left="download"
          @click="isFileUploadModalActive = true"
        >
          Load Shared Database
        </b-button>
      </div>
    </HeroBar>

    <section class="section is-main-section">
      <Tiles :max-per-row="2">
        <CardComponent
          v-for="(db, dbIndex) in databases"
          :key="dbIndex"
          :title="db.name"
          :icon="db.type !== LOCAL_DB_TYPE ? 'database-export' : 'database'"
          class="tile is-child"
        >
          <template #header>
            <div class="card-header-title level-right">
              <b-button
                title="Delete Database"
                type="is-ghost"
                @click.prevent="deleteModalOpen({dbName: db.name})"
              >
                <b-icon
                  icon="delete"
                />
              </b-button>
            </div>
          </template>
          <b-field :label="'Number of Tables: ' + db.tables.length" />
          <b-field :label="'Category: ' + `${db.type !== LOCAL_DB_TYPE ? 'Shared' : 'Local'}`" />
          <div class="level-left">
            <b-button
              type="is-light"
              class="mr-3"
              tag="router-link"
              :to="{name:'createTable', params: {dbName: db.name, dbType: db.type}}"
            >
              <strong>Create Table</strong>
            </b-button>
            <b-button
              v-if="db.type !== LOCAL_DB_TYPE"
              type="is-light"
              class="mr-3"
              @click="createInvite({dbName: db.name})"
            >
              <strong>Create Invite</strong>
            </b-button>
            <b-button
              v-if="db.type !== LOCAL_DB_TYPE"
              type="is-light"
              class="mr-3"
              @click="loadSharedTables(db.name)"
            >
              <strong>Load all Tables</strong>
            </b-button>
            <b-button
              v-if="db.type !== LOCAL_DB_TYPE"
              type="is-light"
              class="mr-3"
              @click="addFederatedTable(db.name)"
            >
              <strong>Add Table</strong>
            </b-button>
            <b-modal
              v-model="isCreateTableModalActive"
              has-modal-card
              trap-focus
              :destroy-on-hide="true"
              aria-role="dialog"
              aria-label="Create Table Modal"
              aria-modal
            >
              <template v-if="isCreateTableModalActive">
                <create-table-modal
                  :database-name="db.name"
                  @close="isCreateTableModalActive = false"
                />
              </template>
            </b-modal>
          </div>
          <hr>
          <div class="table-container level-left customTable">
            <table class="table is-narrow is-hoverable is-fullwidth">
              <tr
                v-for="(table, tableIndex) in db.tables"
                :key="tableIndex"
              >
                <th id="tableName">
                  <router-link :to="{path: `/databases/${db.name}/tables/${table.name}`}">
                    <p class="py-2">
                      <b-icon
                        :icon="table.isShared ? 'table-eye' : 'table'"
                        custom-size="default"
                      />
                      {{ table.name }}
                    </p>
                  </router-link>
                </th>
                <td>
                  <div class="buttons is-right">
                    <b-button
                      v-if="db.type !== LOCAL_DB_TYPE"
                      title="Create Table Invite"
                      type="is-ghost"
                      @click.prevent="createInvite({dbName: db.name, tableName: table.name})"
                    >
                      <b-icon
                        :icon="'table-arrow-right'"
                      />
                    </b-button>
                    <b-button
                      title="Delete Table"
                      type="is-ghost"
                      @click.prevent="deleteModalOpen({dbName: db.name, tableName: table.name})"
                    >
                      <b-icon
                        icon="delete"
                      />
                    </b-button>
                  </div>
                </td>
              </tr>
              <tr
                v-for="(unloadedTable, undloadedTableIndex) in db.unloadedTables"
                :key="'undloadedTable'+undloadedTableIndex"
              >
                <th id="tableName">
                  <p class="py-2 not-loaded">
                    <b-icon
                      :icon="'table-eye'"
                      custom-size="default"
                    />
                    {{ unloadedTable }}
                  </p>
                </th>
                <td>
                  <div class="buttons is-right no-wrap">
                    <b-button
                      title="Load Table"
                      type="is-ghost"
                      @click.prevent="loadSharedTable(db.name, unloadedTable)"
                    >
                      <b-icon
                        icon="download"
                      />
                    </b-button>
                  </div>
                </td>
              </tr>
            </table>
          </div>
        </CardComponent>
      </Tiles>
      <DeleteModal
        :is-active="isDeleteModalActive"
        :title="deleteObject ? deleteObject.title : null"
        :db-name="deleteObject ? deleteObject.dbName : null"
        :table-name="deleteObject ? deleteObject.tableName : null"
        :text="deleteObject ? deleteObject.text : null"
        @confirm="deleteConfirm"
        @cancel="deleteCancel"
      />
      <b-modal
        v-model="isInviteModalActive"
        trap-focus
        width="60%"
        :destroy-on-hide="true"
        aria-role="dialog"
        aria-label="Create Invite File"
        aria-modal
      >
        <template v-if="isInviteModalActive">
          <InviteModal
            :db-name="dbName"
            :table-name="tableName ? tableName : null"
            @on-cancel="isInviteModalActive = false"
            @close="isInviteModalActive = false"
          />
        </template>
      </b-modal>
      <b-modal
        v-model="isCreateDatabaseModalActive"
        has-modal-card
        trap-focus
        :destroy-on-hide="false"
        aria-role="dialog"
        aria-label="Create Database Modal"
        aria-modal
      >
        <template v-if="isCreateDatabaseModalActive">
          <CreateDatabaseModal
            :fetch-data="fetchData"
            @close="isCreateDatabaseModalActive = false"
          />
        </template>
      </b-modal>
      <b-modal
        v-model="isFileUploadModalActive"
        has-modal-card
        trap-focus
        :destroy-on-hide="false"
        aria-role="dialog"
        aria-label="Invite Upload Modal"
        aria-modal
      >
        <template v-if="isFileUploadModalActive">
          <FileUploadModal
            :table="tableName"
            :database="dbName"
            :is-federated="isFederated"
            @close="isFileUploadModalActive = false"
          />
        </template>
      </b-modal>
      <b-loading
        v-model="isLoading"
        :can-cancel="false"
      />
    </section>
  </div>
</template>

<script>
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'
import Tiles from '@/components/Tiles'
import CardComponent from '@/components/CardComponent'
import axios from 'axios'
import FileUploadModal from '@/components/FileUploadModal.vue'
import CreateDatabaseModal from '@/components/CreateDatabaseModal.vue'
import DeleteModal from '@/components/DeleteModal.vue'
import InviteModal from '@/components/InviteModal.vue'

export default {
  name: 'Databases',
  components: {
    HeroBar,
    TitleBar,
    Tiles,
    CardComponent,
    FileUploadModal,
    CreateDatabaseModal,
    DeleteModal,
    InviteModal
  },
  data () {
    return {
      HISTORY_TABLE_PREFIX: 'history_',
      SYSTEM_TABLES: ['meta_table', 'data_chains', 'shared_tables', 'key_store'],
      SYSTEM_DATABASES: ['information_schema', 'mysql', 'performance_schema', 'sys', 'trustdble'],
      LOCAL_DB_TYPE: 'LOCAL',
      databases: [],
      dbName: null,
      tableName: null,
      deleteObject: null,
      isFileUploadModalActive: false,
      isCreateTableModalActive: false,
      isCreateDatabaseModalActive: false,
      isDeleteModalActive: false,
      isLoading: false,
      isInviteModalActive: false,
      isFederated: false
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE',
        'Databases'
      ]
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
    async loadSharedTables (dbName) {
      try {
        await axios.post(`api/databases/${dbName}/tables/load`)
        this.fetchData()
      } catch (error) {
        this.showErrorPrompt(`Error: ${error.message}`)
      }
    },
    async loadSharedTable (dbName, tableName) {
      try {
        await axios.post(`api/databases/${dbName}/tables/${tableName}/load`)
        this.fetchData()
      } catch (error) {
        if (error.response.data.code === 'ER_KEY_NOT_FOUND') {
          this.dbName = dbName
          this.tableName = tableName
          this.isFileUploadModalActive = true
        } else {
          this.showErrorPrompt(error)
        }
      }
    },
    addFederatedTable (dbName) {
      this.dbName = dbName
      this.isFederated = true
      this.isFileUploadModalActive = true
    },
    async createInvite (obj) {
      this.dbName = obj.dbName
      this.tableName = obj.tableName ? obj.tableName : null
      this.isInviteModalActive = true
    },
    deleteModalOpen (obj) {
      obj.title = `Delete ${obj.tableName ? 'Table' : 'Database'}`
      obj.text = `Are you sure? This action will delete
        ${obj.tableName ? 'table \'' + obj.tableName + '\' from' : ''}
        database '${obj.dbName}' permanently.`
      this.deleteObject = obj
      this.isDeleteModalActive = true
    },
    async deleteConfirm () {
      this.isDeleteModalActive = false
      try {
        this.isLoading = true
        await axios.delete(
          `api/databases/${this.deleteObject.dbName}${this.deleteObject.tableName
          ? '/tables/' + this.deleteObject.tableName
          : ''}`
        )
        this.messagePrompt({
          message: `${this.deleteObject.tableName ? 'Table' : 'Database'} successfully deleted`,
          type: 'is-success'
        })
        this.fetchData()
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    },
    deleteCancel () {
      this.isDeleteModalActive = false
    }
  }
}
</script>

<style scoped>
  .not-loaded {
    opacity: 0.5;
  }
</style>
