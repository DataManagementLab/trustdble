<template>
  <div>
    <TitleBar :title-stack="titleStack" />

    <HeroBar>
      <router-link :to="{path: `/databases/${dbName}/tables/${tableName}`}">
        <b-icon
          icon="chevron-left"
          custom-size="default"
        />
      </router-link>
      Entry History
    </HeroBar>

    <section class="section is-main-section">
      <b-field>Primary Key Hash: {{ primaryKeyHash }}</b-field>
      <b-pagination
        :total="totalBlocks"
        :current.sync="currentPage"
        :per-page="maxBlocksPerPage"
      />

      <Tiles :max-per-row="maxBlocksPerRow">
        <CardComponent
          v-for="(block, index) in paginatedBlocks"
          :key="index"
          :title="`Block: ${block.blockNumber}`"
          icon="package-variant-closed"
          class="tile is-child"
        >
          <template #header>
            <div class="card-header-title level-right">
              <p
                v-if="block.blockNumber === blocks[0].blockNumber"
                :style="{'color':'green'}"
              >
                Create
              </p>
              <p
                v-else-if="block.transactionType === 'REMOVE'"
                :style="{'color':'red'}"
              >
                Remove
              </p>
              <p
                v-else
                :style="{'color':'orange'}"
              >
                Change
              </p>
            </div>
          </template>
          <div class="table-container level-left">
            <table class="table is-narrow is-fullwidth">
              <div v-if="block.transactionType === 'PUT'">
                <tr
                  v-for="(label) of columnNames"
                  :key="label"
                >
                  <th>{{ label }}</th>
                  <td>{{ block.values[label] }}</td>
                </tr>
              </div>
              <tr>
                <th>Timestamp: </th>
                <td>{{ block.timestamp }}</td>
              </tr>
              <tr>
                <th>DataChain ID: </th>
                <td>{{ block.dataChainId }}</td>
              </tr>
              <tr>
                <th>TransactionID: </th>
                <td>{{ block.transactionID }}</td>
              </tr>
            </table>
          </div>
          <b-button
            type="is-ghost"
            @click="showEditorDetails(block.editor)"
          >
            Editor: {{ block.editor }}
          </b-button>
        </CardComponent>
      </Tiles>
    </section>
  </div>
</template>

<script>
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar.vue'
import CardComponent from '@/components/CardComponent.vue'
import axios from 'axios'
import Tiles from '../components/Tiles.vue'

export default {
  name: 'EntryDetails',
  components: { HeroBar, TitleBar, CardComponent, Tiles },
  props: {
    dbName: {
      type: String,
      default: null
    },
    tableName: {
      type: String,
      default: null
    },
    primaryKeyHash: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      HISTORY_TABLE_PREFIX: 'history_',
      blocks: [],
      columnNames: [],
      currentPage: 1,
      maxBlocksPerPage: 6,
      maxBlocksPerRow: 2
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE',
        this.dbName,
        this.tableName
      ]
    },
    totalBlocks () {
      return this.blocks.length
    },
    paginatedBlocks () {
      const currentPageNumber = this.currentPage - 1
      return this.blocks.slice(currentPageNumber * this.maxBlocksPerPage, (currentPageNumber + 1) * this.maxBlocksPerPage)
    }
  },
  async created () {
    this.columnNames = await this.fetchColumnNames()
    this.blocks = await this.fetchEntryHistory()
  },
  methods: {
    async fetchColumnNames () {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}`)
        return response.data.columns.map(column => column.name)
      } catch (error) {
        this.showErrorPrompt(error)
        return []
      }
    },
    async fetchEntryHistory () {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}/history/tuples/${this.primaryKeyHash}/changes`)
        return response.data.tuple_states.map(state => ({
          timestamp: state.datetime,
          transactionID: state.transactionId,
          transactionType: state.transactionType,
          blockNumber: state.blockNumber,
          dataChainId: state.dataChainId,
          editor: state.editor,
          values: state.dataValue
            ? Object.fromEntries(state.dataValue.split(',').map((val, index) => [this.columnNames[index], val]))
            : []
        }))
      } catch (error) {
        this.showErrorPrompt(error)
        return []
      }
    },
    showEditorDetails (editor) {
      this.$router.push({ path: `/databases/${this.dbName}/tables/${this.tableName}/history/editors/${editor}/changes` })
    }
  }
}
</script>
