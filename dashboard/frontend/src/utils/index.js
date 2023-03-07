import Vue from 'vue'
import { ToastProgrammatic as Toast, SnackbarProgrammatic as Snackbar, NotificationProgrammatic as Notification } from 'buefy'

Vue.mixin({
  methods: {
    showErrorPrompt: (error) => {
      Toast.open({
        message: `${error.response.data.code}: ${error.response.data.message}`,
        type: 'is-danger'
      }, 3000)
    },
    messagePrompt: (messageObj) => {
      Toast.open({
        message: messageObj.message,
        type: messageObj.type
      }, 3000)
    },
    showInvalidInputErrorPrompt: (message) => {
      Snackbar.open({
        message,
        queue: false,
        type: 'is-danger'
      }, 3000)
    },
    showSuccessPrompt: (message) => {
      Notification.open({
        message,
        queue: false,
        position: 'is-bottom-right',
        type: 'is-success'
      }, 3000)
    },
    showWarnPrompt: (message) => {
      Notification.open({
        message,
        queue: false,
        position: 'is-bottom-right',
        type: 'is-warning'
      }, 3000)
    }
  }
})
