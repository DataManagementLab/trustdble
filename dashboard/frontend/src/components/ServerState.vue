<template>
  <span>
    <span v-if="isRunning">
      <strong><span style="color:#26A001;">&#8226;</span></strong>
      <span> Server is running</span>
    </span>
    <span v-if="!isRunning">
      <strong><span style="color:#FF3B3B;">&#8226;</span></strong>
      <span> Server is stopped</span>
    </span>
  </span>
</template>

<script>
import axios from 'axios'

export default {
  name: 'ServerState',
  data () {
    return {
      isRunning: false
    }
  },
  watch: {
    $route: 'checkServer'
  },
  mounted () {
    this.checkServer()
  },
  methods: {
    async checkServer () {
      try {
        await axios.get('/api/sqlServer/status')
        this.isRunning = true
      } catch (error) {
        this.isRunning = false
      }
    }
  }
}
</script>
