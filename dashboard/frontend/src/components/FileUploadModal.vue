<template>
  <div
    class="modal-card"
    style="width: auto"
  >
    <header class="modal-card-head">
      <p class="modal-card-title">
        Upload Invite File for a {{ database ? 'table' : 'database' }}
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
              <p>Drop your invite file here or click to select</p>
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
      <div
        v-if="dbName"
      >
        <p> <b>Database Name: </b>  {{ dbName }} </p>
      </div>
      <div
        v-if="tableName"
      >
        <p> <b>Table Name: </b>  {{ tableName }} </p>
      </div>
      <div
        v-if="file && (!dbName || ((table !== tableName) && !isFederated) || (database && (database !== dbName)))"
      >
        <div v-if="!table && tableName && !isFederated">
          <p style="color:red">
            This Invite file is not related to a database !
          </p>
        </div>
        <div v-else-if="table && !tableName">
          <p style="color:red">
            This Invite file is not related to a table !
          </p>
        </div>
        <div v-else-if="database && dbName && database !== dbName">
          <p style="color:red">
            This Invite file is related to database  <strong style="color:red">{{ dbName }}</strong>
            but not related to database <strong style="color:red">{{ database }}</strong> !
          </p>
        </div>
        <div v-else-if="table && table !== tableName">
          <p style="color:red">
            This Invite file is related to table  <strong style="color:red">{{ tableName }}</strong>
            but not related to table <strong style="color:red">{{ table }}</strong> !
          </p>
        </div>
        <div v-else>
          <p style="color:red">
            Invalid invite file !!
          </p>
          <p style="color:red">
            Please select a valid invite file
          </p>
        </div>
      </div>
    </section>
    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('close')"
      />
      <b-button
        label="Upload"
        type="is-primary"
        :disabled="!file || (!dbName || ((table !== tableName) && !isFederated) || (database && (database !== dbName)))"
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
  name: 'FileUploadModal',
  props: {
    table: {
      type: String,
      default: null
    },
    database: {
      type: String,
      default: null
    },
    isFederated: {
      type: Boolean,
      default: false
    }
  },
  data () {
    return {
      file: null,
      isLoading: false,
      isFullPage: true,
      dbName: null,
      tableName: null,
      inviteFileContent: null
    }
  },
  methods: {
    getFileContent () {
      const fileReader = new FileReader()
      fileReader.readAsBinaryString(this.file)
      fileReader.onloadend = async () => {
        this.inviteFileContent = JSON.parse(fileReader.result)
        this.dbName = this.inviteFileContent.dbName ? this.inviteFileContent.dbName : null
        this.tableName = this.inviteFileContent.tableName ? this.inviteFileContent.tableName : null
        delete this.inviteFileContent.tableName
        delete this.inviteFileContent.dbName
      }
    },
    async upload () {
      this.isLoading = true
      if (!this.table && !this.isFederated) {
        try {
          await axios.post(`api/databases/${this.dbName}/add`, { inviteFileContent: this.inviteFileContent })
          this.messagePrompt({ message: 'Database added successfully', type: 'is-success' })
          this.$emit('close')
          this.$router.go()
        } catch (error) {
          this.showErrorPrompt(error)
        } finally {
          this.isLoading = false
        }
      } else {
        try {
          await axios.post(`api/databases/${this.database}/tables/${this.tableName}/add`, { inviteFileContent: this.inviteFileContent })
          if (this.table) { await axios.post(`api/databases/${this.database}/tables/${this.table}/load`) }
          this.messagePrompt({ message: 'Table loaded successfully', type: 'is-success' })
          this.$emit('close')
          this.$router.go()
        } catch (error) {
          this.showErrorPrompt(error)
        } finally {
          this.isLoading = false
        }
      }
    }
  }
}
</script>
