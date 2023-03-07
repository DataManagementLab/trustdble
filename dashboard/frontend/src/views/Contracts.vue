<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar>
      Data Contracts
    </HeroBar>

    <section class="section is-main-section">
      <CardComponent
        class="has-table has-mobile-sort-spaced"
        :title="'My Contracts'"
        :icon="'account-arrow-left'"
      >
        <div>
          <b-table
            :data="data"
            height="max-content"
            class="heightAdjustment"
          >
            <b-table-column
              v-for="(column, index) in columns"
              v-slot="props"
              :key="index"
              :field="column.field"
              :label="column.label"
              sortable
            >
              {{ props.row[`${column.field}`] }}
            </b-table-column>
            <b-table-column
              v-slot="props"
            >
              <b-button
                title="Create Contract Invite"
                type="is-ghost"
                @click.prevent="isContractInviteModalActive = true, name =props.row.name"
              >
                <b-icon
                  icon="download"
                />
              </b-button>
              <b-modal
                v-model="isContractInviteModalActive"
                has-modal-card
                trap-focus
                :destroy-on-hide="true"
                aria-role="dialog"
                aria-label="Create Data Contract Invite"
                aria-modal
              >
                <template v-if="isContractInviteModalActive">
                  <InviteContractModal
                    :dcname="name"
                    @on-cancel="isContractInviteModalActive = false"
                    @close="isContractInviteModalActive = false, fetchData(true)"
                  />
                </template>
              </b-modal>
            </b-table-column>
            <b-table-column
              v-slot="props"
            >
              <b-button
                title="Delete Data Contract"
                type="is-ghost"
                @click.prevent="deleteDataContract(props.row.name)"
              >
                <b-icon
                  icon="delete"
                />
              </b-button>
            </b-table-column>
          </b-table>
        </div>
      </CardComponent>

      <div class="buttons is-left">
        <b-button
          class="button is-link"
          icon-left="account-arrow-left"
          @click="isCreateContractModalActive = true"
        >
          Create Contract
        </b-button>
        <b-modal
          v-model="isCreateContractModalActive"
          has-modal-card
          trap-focus
          :destroy-on-hide="true"
          aria-role="dialog"
          aria-label="Create Data Contract"
          aria-modal
        >
          <template v-if="isCreateContractModalActive">
            <CreateContractModal
              @on-cancel="isCreateContractModalActive = false"
              @close="isCreateContractModalActive = false, fetchData(true)"
            />
          </template>
        </b-modal>
      </div>

      <CardComponent
        class="has-table has-mobile-sort-spaced"
        :title="'Remote Contracts'"
        :icon="'account-arrow-left'"
      >
        <div>
          <b-table
            :data="remote_data"
            height="max-content"
            class="heightAdjustment"
          >
            <b-table-column
              v-for="(column, index) in remote_columns"
              v-slot="props"
              :key="index"
              :field="column.field"
              :label="column.label"
              sortable
            >
              {{ props.row[`${column.field}`] }}
            </b-table-column>
            <b-table-column
              v-slot="props"
              field="result"
              label="result"
            >
              {{ result[props.row.name] }}
            </b-table-column>
            <b-table-column
              v-slot="props"
            >
              <b-button
                :loading="isLoading[props.row.name]"
                title="Execute Data Contract"
                type="is-ghost"
                @click.prevent="isContractExecuteModalActive = true, name =props.row.name"
              >
                <b-icon
                  icon="motion-play-outline"
                />
              </b-button>
              <b-modal
                v-model="isContractExecuteModalActive"
                has-modal-card
                trap-focus
                :destroy-on-hide="true"
                aria-role="dialog"
                aria-label="Execute Data Contract"
                aria-modal
              >
                <template v-if="isContractExecuteModalActive">
                  <ExecuteContractModal
                    :dcname="name"
                    @on-cancel="isContractExecuteModalActive = false"
                    @parameters="processResult"
                  />
                </template>
              </b-modal>
            </b-table-column>
            <b-table-column
              v-slot="props"
            >
              <b-button
                title="Delete Data Contract"
                type="is-ghost"
                @click.prevent="deleteRemoteDataContract(props.row.name)"
              >
                <b-icon
                  icon="delete"
                />
              </b-button>
            </b-table-column>
          </b-table>
        </div>
      </CardComponent>

      <div class="buttons is-left">
        <b-button
          class="button is-link"
          icon-left="plus"
          @click="isContractUploadModalActive = true"
        >
          Add Contract
        </b-button>
        <b-modal
          v-model="isContractUploadModalActive"
          has-modal-card
          trap-focus
          :destroy-on-hide="true"
          aria-role="dialog"
          aria-label="Add Data Contract"
          aria-modal
        >
          <template v-if="isContractUploadModalActive">
            <AddContractModal
              @on-cancel="isContractUploadModalActive = false"
              @close="isContractUploadModalActive = false, fetchData(true)"
            />
          </template>
        </b-modal>
      </div>
    </section>
  </div>
</template>

<script>
import CardComponent from '@/components/CardComponent'
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'
import CreateContractModal from '@/components/CreateContractModal.vue'
import AddContractModal from '@/components/AddContractModal.vue'
import InviteContractModal from '@/components/InviteContractModal.vue'
import ExecuteContractModal from '@/components/ExecuteContractModal.vue'
import axios from 'axios'

export default {
  name: 'Contracts',
  components: { HeroBar, TitleBar, CardComponent, CreateContractModal, AddContractModal, InviteContractModal, ExecuteContractModal },
  props: {
  },
  data () {
    return {
      tableComponentKey: 0,
      response_data: null,
      data: [],
      columns: [],
      result: {},
      isLoading: {},
      isCreateContractModalActive: false,
      isContractUploadModalActive: false,
      isContractInviteModalActive: false,
      isContractExecuteModalActive: false
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE',
        'Data Contracts'
      ]
    }
  },
  created () {
    this.fetchData(false)
  },
  methods: {
    async fetchData (isUpdate) {
      this.response_data = await this.fetchDataContractTable()
      this.response_remote_data = await this.fetchRemoteDataContractTable()
      this.data = []
      this.columns = []
      this.remote_data = []
      this.remote_columns = []
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
      this.response_remote_data.forEach((value, key) => {
        const keys = Object.keys(this.response_remote_data[key])
        const values = Object.values(this.response_remote_data[key])
        var object = {}
        object.Id = key + 1
        keys.forEach((val, index) => {
          const presence = this.remote_columns.findIndex(obj => obj.label === val)
          if (presence === -1) {
            this.remote_columns.push(
              {
                field: `${val}`,
                label: `${val}`
              }
            )
          }
          object[`${val}`] = values[index]
        })
        this.remote_data.push(object)
      })
    },
    async fetchDataContractTable () {
      try {
        const response = await axios.get('/api/datacontracts/local')
        return response.data
      } catch (error) {
        this.showErrorPrompt(error)
        return null
      }
    },
    async fetchRemoteDataContractTable () {
      try {
        const response = await axios.get('/api/datacontracts/remote')
        return response.data
      } catch (error) {
        this.showErrorPrompt(error)
        return null
      }
    },
    async deleteDataContract (name) {
      try {
        const response = await axios.delete(`/api/datacontracts/${name}`)
        this.showWarnPrompt(`Contract ${name} deleted`)
        return response.data
      } catch (error) {
        this.showErrorPrompt(error)
        return null
      } finally {
        this.fetchData()
      }
    },
    async deleteRemoteDataContract (name) {
      try {
        const response = await axios.delete(`/api/datacontracts/remote/${name}`)
        this.showWarnPrompt(`Contract ${name} deleted`)
        return response.data
      } catch (error) {
        this.showErrorPrompt(error)
        return null
      } finally {
        this.fetchData()
      }
    },
    async processResult (parameters, name) {
      this.isContractExecuteModalActive = false
      this.isLoading[name] = true
      if (parameters === null) parameters = 'None'
      try {
        const response = await axios.post(`api/datacontracts/${name}/execute`, { parameterList: parameters })
        this.result[name] = response.data.result
        this.showSuccessPrompt('Data contract execution successful!')
      } catch (error) {
        this.showWarnPrompt('Data contract execution failed!')
      } finally {
        this.isLoading[name] = false
        this.fetchData()
      }
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
