<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar :has-right-visible="false">
      Overview
    </HeroBar>
    <section class="section is-main-section">
      <Tiles>
        <CardWidget
          class="tile is-child"
          type="is-primary"
          icon="database"
          :number="databaseCount"
          label="Databases"
        />
        <CardWidget
          class="tile is-child"
          type="is-primary"
          icon="table"
          :number="tableCount"
          label="Tables"
        />
      </Tiles>
    </section>
  </div>
</template>

<script>
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'
import Tiles from '@/components/Tiles'
import CardWidget from '@/components/CardWidget'
import axios from 'axios'

export default {
  name: 'Home',
  components: {
    CardWidget,
    Tiles,
    HeroBar,
    TitleBar
  },
  data () {
    return {
      HISTORY_TABLE_PREFIX: 'history_',
      META_TABLE_NAME: 'meta_table',
      SYSTEM_DATABASES: ['information_schema', 'mysql', 'performance_schema', 'sys', 'trustdble'],
      databaseCount: 0,
      tableCount: 0
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE'
      ]
    }
  },
  created () {
    this.fetchMetadata()
  },
  methods: {
    async fetchMetadata () {
      let databases = await this.fetchDatabases()
      databases = databases.filter(db => !this.SYSTEM_DATABASES.includes(db.name))
      this.databaseCount = databases.length
      for (const db of databases) {
        this.tableCount += await this.fetchTables(db.name)
      }
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
        const response = await axios.get(`api/databases/${dbName}`)
        return response.data.numTables - response.data.numHistoryTables - response.data.numMetaTable
      } catch (error) {
        this.showErrorPrompt(error)
        return 0
      }
    }
  }
}
</script>
