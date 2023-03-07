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
      <b-field
        label="Enter your Public Key"
      >
        <b-input
          v-model="publicKey"
          maxlength="1000"
          type="textarea"
        />
      </b-field>
    </section>
    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('on-cancel')"
      />
      <b-button
        label="Create"
        type="is-primary submit"
        :disabled="isDisabled"
        @click="encryptContract()"
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
  name: 'InsertPublicKeyModal',
  props: {
    dcname: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      publicKey: null,
      isLoading: false,
      isDisabled: true,
      message: `Create Invite File for ${this.dcname}`
    }
  },
  watch: {
    publicKey (newvalue, oldvalue) {
      if (newvalue.length > 0) {
        this.isDisabled = false
      } else {
        this.isDisabled = true
      }
    }
  },
  methods: {
    async encryptContract () {
      try {
        this.isLoading = true
        const response = await axios.post(`api/datacontracts/${this.dcname}/invite`, { publicKey: this.publicKey.replace(/[\r\n]+/gm, '') })
        const encryptedDC = response.data.encryptedDC
        const filename = `${this.dcname}_datacontracts.json`
        const element = document.createElement('a')
        element.setAttribute('href', 'data:application/json;charset=utf-8,' + encodeURIComponent(JSON.stringify(encryptedDC)))
        element.setAttribute('download', filename)
        element.style.display = 'none'
        document.body.appendChild(element)

        element.click()
        document.body.removeChild(element)
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
        this.$emit('on-cancel')
      }
    }
  }
}
</script>
