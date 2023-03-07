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
          <b>{{ title }}</b>
        </p>
        <button
          type="button"
          class="delete"
          @click="cancel"
        />
      </header>
      <section class="modal-card-body">
        <p>{{ text }}</p>
      </section>
      <footer class="modal-card-foot  is-light">
        <b-button
          @click="cancel"
        >
          Cancel
        </b-button>
        <b-button
          type="is-danger"
          @click="confirm"
        >
          Delete
        </b-button>
      </footer>
    </div>
  </b-modal>
</template>

<script>

export default {
  name: 'DeleteModal',
  props: {
    isActive: {
      type: Boolean,
      default: false
    },
    title: {
      type: String,
      default: null
    },
    text: {
      type: String,
      default: null
    }
  },
  emits: ['cancel', 'confirm'],
  methods: {
    confirm () {
      this.$emit('confirm')
    },
    cancel () {
      this.$emit('cancel')
    }
  }
}
</script>
