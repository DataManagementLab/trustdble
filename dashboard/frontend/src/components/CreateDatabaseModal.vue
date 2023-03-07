<template>
  <div
    class="modal-card"
    style="width: auto"
  >
    <header class="modal-card-head">
      <p class="modal-card-title">
        Create Database
      </p>
      <button
        type="button"
        class="delete"
        @click="$emit('close')"
      />
    </header>

    <section class="modal-card-body">
      <b-field label="Name">
        <b-input
          v-model="dbName"
          type="text"
          placeholder="Database Name"
          required
        />
      </b-field>
      <b-field :label="'Type'">
        <b-select
          v-model="dbType"
          expanded
        >
          <option
            v-for="(option, index) in dbTypes"
            :key="index"
          >
            {{ option }}
          </option>
        </b-select>
      </b-field>
      <b-field
        v-if="dbType == 'ETHEREUM' || dbType == 'STUB'"
        label="Shards"
      >
        <b-input
          v-model="shards"
          placeholder="Number of Shards"
          type="number"
          min="1"
          :value="shards"
        />
      </b-field>
    </section>

    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('close')"
      />
      <b-button
        label="Confirm"
        type="is-primary"
        @click="createDatabase"
      />
    </footer>

    <b-loading
      v-model="isLoading"
      :can-cancel="false"
    />
  </div>
</template>

<script>
import axios from 'axios'

export default {
  name: 'CreateDatabaseModal',
  props: {
    fetchData: {
      type: Function,
      default: null
    }
  },
  data () {
    return {
      dbTypes: ['LOCAL', 'STUB', 'ETHEREUM'],
      dbName: '',
      dbType: '',
      shards: 1,
      isLoading: false
    }
  },
  methods: {
    async createDatabase () {
      if (!this.validateInput()) {
        this.showInvalidInputErrorPrompt('Please fill out all fields')
        return
      }
      this.isLoading = true
      const db = { dbName: this.dbName, dbType: this.dbType, shards: parseInt(this.shards) }
      try {
        await axios.post('api/databases', db)
        this.messagePrompt({ message: `Database ${this.dbName} created`, type: 'is-success' })
        this.$emit('close')
        this.fetchData()
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    },
    validateInput () {
      let isValid = true
      if (!this.dbName || !this.dbType || !this.shards) {
        isValid = false
      }
      return isValid
    }
  }
}
</script>
