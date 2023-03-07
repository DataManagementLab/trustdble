
<template>
  <div
    class="modal-card"
    style="width: auto"
  >
    <header class="modal-card-head">
      <p class="modal-card-title">
        Upload File for add a new Contract
      </p>
      <button
        type="button"
        class="delete"
        @click="$emit('close')"
      />
    </header>
    <section class="modal-card-body">
      <b-field
        class="file is-primary"
        :class="{'has-name': !!file}"
      >
        <b-upload
          v-model="file"
          class="file-label"
          accept="application/json"
          drag-drop
          expanded
          @change.native="getFileContent"
        >
          <section class="section">
            <div class="content has-text-centered">
              <p>
                <b-icon
                  icon="upload"
                  size="is-large"
                />
              </p>
              <p>Drop your contract file here or click to select</p>
            </div>
          </section>
          <span
            v-if="file"
            class="file-name"
          >
            {{ file.name }}
          </span>
        </b-upload>
      </b-field>
    </section>
    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('close')"
      />
      <b-button
        label="Upload"
        type="is-primary"
        @click="upload"
      />
    </footer>
    <b-loading
      v-model="isLoading"
      :is-full-page="isFullPage"
      :can-cancel="false"
    />
  </div>
</template>

<script>
import axios from 'axios'

export default {
  name: 'AddContractModal',
  props: {
  },
  data () {
    return {
      file: null,
      isLoading: false,
      isFullPage: true,
      encryptedDC: null
    }
  },
  methods: {
    getFileContent () {
      const fileReader = new FileReader()
      fileReader.readAsBinaryString(this.file)
      fileReader.onloadend = async () => {
        this.encryptedDC = JSON.parse(fileReader.result)
      }
    },
    async upload () {
      this.isLoading = true
      try {
        await axios.post('api/datacontracts/add', { encryptedDC: this.encryptedDC })
        this.showSuccessPrompt('Contract successfully added!')
        this.$emit('close')
      } catch (error) {
        this.showErrorPrompt(error)
      } finally {
        this.isLoading = false
      }
    }
  }
}
</script>
