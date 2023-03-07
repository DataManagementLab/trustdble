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
        label="Enter your Parameter List"
      >
        <b-input
          v-model="parameters"
          maxlength="100"
          type="text"
        />
      </b-field>
    </section>
    <footer class="modal-card-foot">
      <b-button
        label="Close"
        @click="$emit('on-cancel')"
      />
      <b-button
        label="Execute"
        type="is-primary submit"
        @click="$emit('parameters', parameters, dcname)"
      />
    </footer>

    <b-loading
      v-model="isLoading"
      :can-cancel="false"
    />
  </div>
</template>

<script>
export default {
  name: 'ExecuteContractModal',
  props: {
    dcname: {
      type: String,
      default: null
    }
  },
  data () {
    return {
      parameters: null,
      isLoading: false,
      isDisabled: true,
      message: `Execute ${this.dcname} contract`
    }
  },
  watch: {
    parameters (newvalue, oldvalue) {
      if (newvalue.length > 0) {
        this.isDisabled = false
      } else {
        this.isDisabled = true
      }
    }
  },
  methods: {

  }
}
</script>
