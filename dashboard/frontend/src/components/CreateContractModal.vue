<template>
  <div
    class="modal-card"
    style="width: auto"
  >
    <header class="modal-card-head">
      <p class="modal-card-title">
        {{ message }}
      </p>
      <button
        type="button"
        class="delete"
        style="margin-left: 10px;"
        @click="$emit('on-cancel')"
      />
    </header>

    <section class="modal-card-body">
      <b-field grouped>
        <b-field
          label="Contract Name"
        >
          <b-input
            v-model="name"
            maxlength="100"
            type="text"
            placeholder="The name (ID) of the contract"
          />
        </b-field>
        <b-field
          label="Owner Name"
        >
          <b-input
            v-model="owner"
            maxlength="100"
            type="text"
            placeholder="The name of the contract owner"
          />
        </b-field>
        <b-field
          label="User Name"
        >
          <b-input
            v-model="user"
            maxlength="100"
            type="text"
            placeholder="The name of the contract user"
          />
        </b-field>
      </b-field>
      <b-field
        grouped
        class="WidthControl"
      >
        <b-field
          label="Enter the logicPath of Contract"
          style="margin-left: 100px;"
        >
          <b-input
            v-model="logicPath"
            maxlength="1000"
            type="textarea"
          />
        </b-field>
        <b-field
          label="Parameter List"
          style="margin-left: 100px;"
        >
          <b-input
            v-model="parameters"
            maxlength="100"
            type="text"
            placeholder="The parameters of the logic"
          />
        </b-field>
      </b-field>
    </section>

    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('on-cancel')"
      />
      <b-button
        label="Create"
        @click="createContract"
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
  name: 'CreateContractModal',
  props: {
  },
  data () {
    return {
      name: null,
      owner: null,
      user: null,
      logicPath: null,
      parameters: null,
      isLoading: false,
      isDisabled: true,
      message: 'Create New Data Contract'
    }
  },
  watch: {
    logicPath (newvalue, oldvalue) {
      if (newvalue.length > 0) {
        this.isDisabled = false
      } else {
        this.isDisabled = true
      }
    },
    name (newvalue, oldvalue) {
      if (newvalue.length > 0) {
        this.isDisabled = false
      } else {
        this.isDisabled = true
      }
    }
  },
  methods: {
    async createContract () {
      try {
        this.isLoading = true
        await axios.post('api/datacontracts', { name: this.name, owner: this.owner, user: this.user, logicPath: this.logicPath, parameters: this.parameters })
        this.showSuccessPrompt(`Contract ${this.name} successfully created!`)
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
        this.$emit('close')
      }
    }
  }
}
</script>
