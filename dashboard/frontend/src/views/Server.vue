<template>
  <div>
    <TitleBar :title-stack="titleStack" />
    <HeroBar>
      Server
    </HeroBar>

    <section class="section is-main-section">
      <Tiles>
        <CardComponent
          title="Server State"
          icon="server"
          class="tile is-child"
        >
          <b-field
            label="Current state:"
            vertical
          >
            <ServerState />
          </b-field>
        </CardComponent>
      </Tiles>
    </section>
  </div>
</template>

<script>
import TitleBar from '@/components/TitleBar'
import HeroBar from '@/components/HeroBar'
import Tiles from '@/components/Tiles'
import CardComponent from '@/components/CardComponent'
import ServerState from '../components/ServerState.vue'

export default {
  name: 'Server',
  components: {
    HeroBar,
    TitleBar,
    Tiles,
    CardComponent,
    ServerState
  },
  data () {
    return {
      isRunning: false
    }
  },
  computed: {
    titleStack () {
      return [
        'TRUSTDBLE',
        'Server'
      ]
    }
  }
}
</script>
