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
        @click="createInvite()"
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
    dbName: {
      type: String,
      default: null
    },
    tableName: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      publicKey: null,
      isLoading: false,
      isDisabled: true,
      message: this.tableName ? `Create Invite File for ${this.dbName}/${this.tableName}` : `Create Invite File for ${this.dbName}`
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
    async createInvite () {
      if (!this.tableName) {
        try {
          this.isLoading = true
          const response = await axios.post(`api/databases/${this.dbName}/invite`, { publicKey: this.publicKey.replace(/[\r\n]+/gm, '') })
          const encryptedInvite = response.data.inviteFileContent
          const filename = `${this.dbName}_invite.json`
          const element = document.createElement('a')
          element.setAttribute('href', 'data:application/json;charset=utf-8,' + encodeURIComponent(JSON.stringify(encryptedInvite)))
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
      } else {
        try {
          this.isLoading = true
          const response = await axios.post(`api/databases/${this.dbName}/tables/${this.tableName}/invite`, { publicKey: this.publicKey.replace(/[\r\n]+/gm, '') })
          const encryptedInvite = {
            dbName: this.dbName,
            tableName: this.tableName,
            invite: response.data.inviteFileContent.invite
          }
          const filename = `${this.dbName}_${this.tableName}_invite.json`
          const element = document.createElement('a')
          element.setAttribute('href', 'data:application/json;charset=utf-8,' + encodeURIComponent(JSON.stringify(encryptedInvite)))
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
}
</script>
