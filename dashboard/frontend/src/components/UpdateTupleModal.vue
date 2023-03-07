<template>
  <b-modal
    :active.sync="isActive"
    has-modal-card
  >
    <div
      class="modal-card"
      style="width: auto"
    >
      <header class="modal-card-head is-light">
        <p class="modal-card-title">
          <b>Edit Values</b>
        </p>
        <button
          type="button"
          class="delete"
          @click="cancel"
        />
      </header>
      <section
        v-if="row"
        class="modal-card-body"
      >
        <b-field
          v-for="(column, index) in Object.keys(row)"
          :key="index"
          :label="column"
        >
          <b-input
            v-model="insiderValues[index]"
          />
        </b-field>
      </section>
      <footer class="modal-card-foot  is-light">
        <b-button
          @click="cancel"
        >
          Cancel
        </b-button>
        <b-button
          type="is-success"
          @click="editValues"
        >
          Edit
        </b-button>
      </footer>
    </div>
    <b-loading
      v-model="isLoading"
      :can-cancel="false"
    />
  </b-modal>
</template>

<script>
import axios from 'axios'

export default {
  name: 'UpdateTupleModal',
  components: {
  },
  props: {
    isActive: {
      type: Boolean,
      default: false
    },
    dbName: {
      type: String,
      default: ''
    },
    tableName: {
      type: String,
      default: ''
    },
    row: {
      type: Object,
      default: null
    },
    selector: {
      type: String,
      default: ''
    }
  },
  emits: ['fetch'],
  data () {
    return {
      insiderValues: [],
      isLoading: false
    }
  },
  watch: {
    isActive (newvalue, oldvalue) {
      if (newvalue) {
        this.makeInsiderValues()
      }
    }
  },
  methods: {
    makeInsiderValues () {
      if (this.row) {
        this.insiderValues = this.row && Object.values(this.row)
      }
    },
    async editValues () {
      let change = false
      let colValues = ''
      this.row && Object.keys(this.row).forEach((columnName, index) => {
        if (this.insiderValues[index] !== this.row[index]) {
          change = true
          colValues += `, ${columnName} = '${this.insiderValues[index]}'`
        }
      })
      if (!change) {
        this.messagePrompt({ message: 'There are no changes in Values', type: 'is-dark' })
        return
      }

      this.isLoading = true
      try {
        await axios.post(`api/databases/${this.dbName}/tables/${this.tableName}/tuples/update`, { colValues: colValues.substring(2), selector: this.selector })
        this.messagePrompt({ message: 'Row Updated successfully', type: 'is-success' })
        this.$emit('fetch')
        this.cancel()
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    },
    cancel () {
      this.$emit('cancel')
    }
  }
}
</script>
