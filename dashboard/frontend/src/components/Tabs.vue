<template>
  <section>
    <b-tabs
      v-model="activeTab"
      :animated="false"
      style="animation: none;"
      :multiline="true"
    >
      <template v-for="tab in tabList">
        <b-tab-item
          :key="tab.content.id"
          style="color:white !important"
          :value="tab.id"
          :label="tab.label"
        >
          <TableView
            v-if="tab.content.isTable"
            :response="tab.content.res"
          />
          <b-input
            v-else
            v-model="tab.content.result"
            maxlength="500"
            class="leftPadding"
            type="textarea"
            disabled
          />
        </b-tab-item>
      </template>
    </b-tabs>
  </section>
</template>

<script>
import TableView from '@/components/TableView'

export default {
  name: 'Tabs',
  components: {
    TableView
  },
  props: {
    baseTabs: {
      type: Array,
      default () {
        return []
      }
    }
  },
  data () {
    return {
      res: null,
      result: 'No output',
      isTable: false,
      activeTab: 'query 1',
      tabList: []
    }
  },
  computed: {
    tabs () {
      const tabs = [...this.baseTabs]
      return tabs
    }

  },
  watch: {
    baseTabs (newvalue, oldvalue) {
      this.activeTab = 'query 1'
      if (newvalue !== oldvalue) {
        this.tabList = [...this.tabs]
        this.tabList.forEach((item, index) => {
          var output = this.handleResponse(item.content)
          this.tabList[index].content = {
            ...this.tabList[index].content,
            ...output
          }
        })
      }
    },
    isTable (value) {
      if (value === true) {
        this.isError = false
      }
    },
    isError (value) {
      if (value === true) {
        this.isTable = false
      }
    },
    result (value) {
      if (value === 'Successful') {
        this.isError = false
        this.isTable = false
      }
    }
  },
  methods: {
    handleResponse (response) {
      if (Object.prototype.toString.call(response) === '[object Array]') {
        if (response.length !== 0) {
          return ({
            res: response,
            isTable: true
          })
        } else {
          return ({
            result: 'Empty',
            isTable: false
          })
        }
      } else {
        return ({
          result: 'Successful',
          isTable: false
        })
      }
    }
  }
}
</script>
