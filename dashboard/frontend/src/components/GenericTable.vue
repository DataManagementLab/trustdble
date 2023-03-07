<template>
  <div>
    <b-table
      :data="tuples"
      height="max-content"
      class="heightAdjustment"
      :default-sort="displayColumns && displayColumns[0] && displayColumns[0].field"
      sticky-header
      striped
      hoverable
    >
      <b-table-column
        v-for="(column, index) in displayColumns"
        v-slot="props"
        :key="index"
        :field="column.field"
        :label="column.label"
        sortable
        searchable
      >
        {{ props.row[`${column.field}`] }}
      </b-table-column>
      <b-table-column
        v-if="(isLatestState && isHistoryExist) || isHistoryExist"
        v-slot="props"
      >
        <b-button
          title="Show History"
          type="is-ghost"
          @click.prevent="showHistoryOfTuple(props.row)"
        >
          <b-icon
            icon="eye"
          />
        </b-button>
      </b-table-column>
      <b-table-column
        v-if="isLatestState"
        v-slot="props"
      >
        <b-button
          title="Update Row"
          type="is-ghost"
          @click.prevent="updateModalOpen({primaryKey, row: props.row })"
        >
          <b-icon
            icon="pencil"
          />
        </b-button>
      </b-table-column>
      <b-table-column
        v-if="isLatestState"
        v-slot="props"
      >
        <b-button
          title="Delete Row"
          type="is-ghost"
          @click.prevent="deleteModalOpen({primaryKey, row: props.row })"
        >
          <b-icon
            icon="delete"
          />
        </b-button>
      </b-table-column>
    </b-table>
    <DeleteModal
      :is-active="isDeleteModalActive"
      :title="deleteObject ? deleteObject.title : null"
      :db-name="dbName"
      :table-name="tableName"
      :text="deleteObject ? deleteObject.text : null"
      @confirm="deleteConfirm"
      @cancel="cancel"
    />
    <UpdateTupleModal
      :is-active="isUpdateModalActive"
      :db-name="dbName"
      :table-name="tableName"
      :row="updateObject ? updateObject.row : null"
      :selector="updateObject ? `${updateObject.primaryKey} = '${updateObject.row[updateObject.primaryKey]}'`: null"
      @cancel="cancel"
      @fetch="fetch"
    />
    <b-loading
      v-model="isLoading"
      :can-cancel="false"
    />
  </div>
</template>

<script>
import DeleteModal from '@/components/DeleteModal.vue'
import UpdateTupleModal from './UpdateTupleModal.vue'
import axios from 'axios'

export default {
  name: 'GenericTable',
  components: {
    DeleteModal,
    UpdateTupleModal
  },
  props: {
    dbName: {
      type: String,
      default: null
    },
    tableName: {
      type: String,
      default: null
    },
    columns: {
      type: Array,
      default: null
    },
    tuples: {
      type: Array,
      default: null
    },
    isSharedTable: {
      type: Boolean,
      default: false
    },
    isLatestState: {
      type: Boolean,
      default: false
    },
    isHistoryExist: {
      type: Number,
      default: 0
    }
  },
  emits: ['fetch'],
  data () {
    return {
      primaryKey: null,
      displayColumns: [],
      isDeleteModalActive: false,
      isUpdateModalActive: false,
      deleteObject: null,
      updateObject: null,
      isLoading: false
    }
  },
  mounted () {
    this.transformColumnsForDisplay()
    this.checkPrimaryKey()
  },
  methods: {
    fetch () {
      this.$emit('fetch')
    },
    deleteModalOpen (obj) {
      obj.title = 'Delete Tuple'
      obj.text = `Are you sure you want to delete the Tuple where ${obj.primaryKey} = ${obj.row[obj.primaryKey]}`
      this.deleteObject = obj
      this.isDeleteModalActive = true
    },
    updateModalOpen (obj) {
      delete obj.row.primaryKeyHash
      this.updateObject = obj
      this.isUpdateModalActive = true
    },
    async deleteConfirm () {
      this.isDeleteModalActive = false
      this.isLoading = true
      try {
        await axios.delete(
          `api/databases/${this.dbName}/tables/${this.tableName}/tuples/delete`,
          { data: { selector: `${this.deleteObject.primaryKey} = '${this.deleteObject.row[this.deleteObject.primaryKey]}'` } })
        this.messagePrompt({ message: 'Row successfully deleted', type: 'is-success' })
        this.fetch()
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    },
    cancel () {
      this.isDeleteModalActive = false
      this.isUpdateModalActive = false
    },
    checkPrimaryKey () {
      this.columns && this.columns.forEach((item) => {
        if (item.isPrimaryKey) {
          this.primaryKey = item.name
        }
      })
      if (!this.primaryKey) {
        this.primaryKey = this.columns && this.columns[0] && this.columns[0].name
      }
    },
    transformColumnsForDisplay () {
      for (const column of this.columns) {
        if (column.name !== 'primaryKeyHash') {
          this.displayColumns.push({ field: column.name, label: column.name })
        }
      }
    },
    showHistoryOfTuple (tuple) {
      if (!this.isSharedTable) {
        return
      }
      const primaryKeyHash = tuple[Object.keys(tuple)[0]]
      this.$router.push({ path: `/databases/${this.dbName}/tables/${this.tableName}/history/tuples/${primaryKeyHash}/changes` })
    }
  }
}
</script>
