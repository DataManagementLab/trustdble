<template>
  <b-table
    class="tableHeight"
    :data="data"
    :striped="true"
    :columns="columns"
    :bordered="true"
  />
</template>

<script>
export default {
  name: 'TableView',
  props: {
    response: {
      type: Array,
      default: null
    }
  },
  data () {
    return {
      response_data: null,
      data: [],
      columns: []
    }
  },
  watch: {
    response (newres, oldres) {
      this.fillData()
    }
  },
  beforeMount () {
    this.fillData()
  },

  methods: {
    fillData () {
      this.data = []
      this.columns = []
      this.response_data = this.response
      this.response_data.forEach((value, key) => {
        const keys = Object.keys(this.response_data[key])
        const values = Object.values(this.response_data[key])
        var object = {}
        object.Id = key + 1
        keys.forEach((val, index) => {
          const presence = this.columns.findIndex(obj => obj.label === val)
          if (presence === -1) {
            this.columns.push(
              {
                field: `${val}`,
                label: `${val}`
              }
            )
          }
          object[`${val}`] = values[index]
        })
        this.data.push(object)
      })
    }
  }
}

</script>
