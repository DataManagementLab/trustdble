<script>
import chunk from 'lodash/chunk'

export default {
  name: 'Tiles',
  props: {
    maxPerRow: {
      type: Number,
      default: 2
    }
  },
  methods: {
    renderAncestor (createElement, elements) {
      return createElement('div', { attrs: { class: 'tile is-ancestor' } }, elements.map((element) => createElement('div', { attrs: { class: 'tile is-parent' } }, [element])))
    }
  },
  render (createElement) {
    if (this.$slots.default <= this.maxPerRow) {
      return this.renderAncestor(createElement, this.$slots.default)
    }
    return createElement('div', { attrs: { class: 'is-tiles-wrapper' } }, chunk(this.$slots.default, this.maxPerRow).map((group) => this.renderAncestor(createElement, group)))
  }
}
</script>
