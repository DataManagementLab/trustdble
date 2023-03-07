<template>
  <div id="app">
    <div class="wrapper">
      <NavBar />
      <AsideMenu
        :menu="menu"
      />
      <router-view />
    </div>
    <FooterBar />
  </div>
</template>

<script>
// @ is an alias to /src
import NavBar from '@/components/NavBar'
import AsideMenu from '@/components/AsideMenu'
import FooterBar from '@/components/FooterBar'

export default {
  name: 'Home',
  components: {
    FooterBar,
    AsideMenu,
    NavBar
  },
  computed: {
    menu () {
      return [
        'General',
        [
          {
            to: '/',
            icon: 'desktop-mac',
            label: 'Overview'
          },
          {
            to: '/databases',
            label: 'Databases',
            icon: 'database'
          },
          {
            to: '/server',
            label: 'Server',
            icon: 'server'
          },
          {
            to: '/blockchains',
            label: 'Blockchains',
            icon: 'cube'
          },
          {
            to: '/contracts',
            label: 'Data Contracts',
            icon: 'account-arrow-left'
          },
          {
            to: '/workbench',
            label: 'SQL Workbench',
            icon: 'database-search'
          }
        ]
      ]
    }
  },
  created () {
    this.$store.commit('user', {
      name: 'John Doe',
      email: 'john@example.com',
      avatar: 'https://avatars.dicebear.com/v2/gridy/John-Doe.svg'
    })
  }
}
</script>
