import Vue from 'vue'
import VueRouter from 'vue-router'
import Home from '../views/Home.vue'
import Databases from '../views/Databases.vue'
import Tables from '../views/Tables.vue'
import Server from '../views/Server.vue'
import Blockchains from '../views/Blockchains.vue'
import CreateTable from '../views/CreateTable.vue'
import EntryDetails from '../views/EntryDetails.vue'
import EditorDetails from '../views/EditorDetails.vue'
import Workbench from '../views/Workbench.vue'
import Contracts from '../views/Contracts.vue'

Vue.use(VueRouter)

const routes = [
  {
    // Document title tag
    // We combine it with defaultDocumentTitle set in `src/main.js` on router.afterEach hook
    meta: {
      title: 'Dashboard'
    },
    path: '/',
    name: 'home',
    component: Home
  },
  {
    meta: {
      title: 'Tables'
    },
    path: '/databases/:dbName/tables/:tableName',
    name: 'tables',
    component: Tables,
    props: route => ({
      dbName: route.params.dbName,
      tableName: route.params.tableName
    })
  },
  {
    meta: {
      title: 'Databases'
    },
    path: '/databases',
    name: 'databases',
    component: Databases
  },
  {
    meta: {
      title: 'Server'
    },
    path: '/server',
    name: 'server',
    component: Server
  },
  {
    meta: {
      title: 'Blockchains'
    },
    path: '/blockchains',
    name: 'blockchains',
    component: Blockchains
  },
  {
    meta: {
      title: 'Data Contracts'
    },
    path: '/contracts',
    name: 'contracts',
    component: Contracts
  },
  {
    meta: {
      title: 'SQL Workbench'
    },
    path: '/workbench',
    name: 'workbench',
    component: Workbench
  },
  {
    meta: {
      title: 'CreateTable'
    },
    path: '/createTable',
    name: 'createTable',
    component: CreateTable,
    props: true
  },
  {
    meta: {
      title: 'EntryDetails'
    },
    path: '/databases/:dbName/tables/:tableName/history/tuples/:primaryKeyHash/changes',
    name: 'entryDetails',
    component: EntryDetails,
    props: route => ({
      dbName: route.params.dbName,
      tableName: route.params.tableName,
      primaryKeyHash: route.params.primaryKeyHash
    })
  },
  {
    meta: {
      title: 'EditorDetails'
    },
    path: '/databases/:dbName/tables/:tableName/history/editors/:editor/changes',
    name: 'editorDetails',
    component: EditorDetails,
    props: route => ({
      dbName: route.params.dbName,
      tableName: route.params.tableName,
      editor: route.params.editor
    })
  }
]

const router = new VueRouter({
  base: process.env.BASE_URL,
  routes,
  scrollBehavior (to, from, savedPosition) {
    if (savedPosition) {
      return savedPosition
    }
    return { x: 0, y: 0 }
  }
})

export default router
