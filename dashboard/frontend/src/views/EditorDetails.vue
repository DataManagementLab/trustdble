<template>
  <div>
    <TitleBar :title-stack="titleStack" />

    <HeroBar>
      <router-link :to="{path:`/databases/${dbName}/tables/${tableName}`}">
        <b-icon
          icon="chevron-left"
          custom-size="default"
        />
      </router-link>
      Editor History
    </HeroBar>

    <section class="section is-main-section">
      <b-field>Editor: {{ editor }}</b-field>

      <b-pagination
        :total="totalBlocks"
        :current.sync="currentPage"
        :per-page="maxBlocksPerPage"
      />

      <Tiles :max-per-row="maxBlocksPerRow">
        <CardComponent
          v-for="(block, index) in paginatedBlocks"
          :key="index"
          :title="`Block: ${block[0].blockNumber}`"
          icon="package-variant-closed"
          class="tile is-child"
        >
          <template #header>
            <div class="card-header-title level-right">
              <p> DataChain: {{ block[0].dataChainId }} </p>
            </div>
          </template>
          <div>
            Primary Key Hash :
            <div
              v-for="(innerBlock, ind) in block"
              :key="ind"
            >
              <b-button
                type="is-ghost"
                @click="showEntryDetails(innerBlock.primaryKeyHash)"
              >
                {{ innerBlock.primaryKeyHash }}
              </b-button>
            </div>
          </div>
        </CardComponent>
      </Tiles>
    </section>
  </div>
</template>

<script>
import TitleBar from '@/components/TitleBar.vue'
import HeroBar from '@/components/HeroBar.vue'
import CardComponent from '@/components/CardComponent.vue'
import Tiles from '../components/Tiles.vue'
import axios from 'axios'

export default {
  name: 'EditorDetails',
  components: { TitleBar, HeroBar, CardComponent, Tiles },
  props: {
    dbName: {
      type: String,
      default: null
    },
    tableName: {
      type: String,
      default: null
    },
    editor: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      HISTORY_TABLE_PREFIX: 'history_',
      blocks: [{}],
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
    groupBlocks () {
      const groupedBlocks = []
      const indexDictionary = {}
      this.blocks.forEach(element => {
        if (Object.values(indexDictionary).findIndex((elem) => elem === `${element.blockNumber}-${element.dataChainId}`) === -1) {
          indexDictionary[Object.keys(indexDictionary).length] = `${element.blockNumber}-${element.dataChainId}`
          groupedBlocks.push([element])
        } else {
          const index = Object.values(indexDictionary).findIndex((elem) => elem === `${element.blockNumber}-${element.dataChainId}`)
          if (groupedBlocks[index] !== undefined) {
            groupedBlocks[index].push(element)
          }
        }
      })
      return groupedBlocks
    },
    paginatedBlocks () {
      const combinedBlocks = this.groupBlocks
      const currentPageNumber = this.currentPage - 1
      return combinedBlocks.slice(currentPageNumber * this.maxBlocksPerPage, (currentPageNumber + 1) * this.maxBlocksPerPage)
    }
  },
  async created () {
    this.blocks = await this.fetchEditorHistory()
  },
  methods: {
    async fetchEditorHistory () {
      try {
        const response = await axios.get(`api/databases/${this.dbName}/tables/${this.tableName}/history/editors/${this.editor}/changes`)
        return response.data.changes
      } catch (error) {
        this.showErrorPrompt(error)
        return []
      }
    },
    showEntryDetails (primaryKeyHash) {
      this.$router.push({ path: `/databases/${this.dbName}/tables/${this.tableName}/history/tuples/${primaryKeyHash}/changes` })
    }
  }
}
</script>
