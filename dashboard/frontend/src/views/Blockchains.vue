<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar>
      Blockchains
    </HeroBar>
  </div>
</template>

<script>
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'

export default {
  name: 'Server',
  components: {
    HeroBar,
    TitleBar
  },
  data () {
    return {
      databases: [
        {
          name: 'myDB',
          tablecount: '3',
          tables: ['Cusomter', 'Orders', 'Payments']
        },
        {
          name: 'test-DB',
          tablecount: '2',
          tables: ['Users', 'Admins']
        },
        {
          name: 'demoDB',
          tablecount: '4',
          tables: ['Table1', 'Table2', 'Table3', 'Table4']
        }
      ]
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE',
        'Blockchains'
      ]
    }
  }
}
</script>
