<template>
<div class="player-cards-list">
  <div v-for="(player,index) in statistics" :key="index">
    <div class="card-player">
        <h4 :class="'card-player-name color-'+ (parseInt(index)+1) ">
          <a v-if="indexedPlayers[index]" :href="`/user/?user_id=${indexedPlayers[index].user_id}`">
            {{replay.players[index].name}}
          </a>
          <template v-else>
            {{replay.players[index].name}}
          </template>
        </h4>
        <div class="player-current-halite">
          {{currentHalite[index]}}
          <span
            class="current-percent"
            :class="currentEfficiency(index) >= 0.5 ? 'green':'red'"
            title="Efficiency (current halite / halite collected so far)">
            {{(currentEfficiency(index) * 100).toFixed(2)}}%
          </span>
        </div>
   <!--     <img class="stats-cube" :src="`/assets/images/visualizer/cube1.png`" alt="cube"> -->
    </div>
    <div class="card-player">
      <div class="chart-box">
        <PlayerHaliteChart :chartData="chartData[index]" :maxY="maxY" :index="frame"/>
      </div>
      <ul class="player-stats-list" v-if="stats">
        <li>
          <label>Ships</label>
          <span>{{stats.frames[frame].players[index].currentShips}}</span>
        </li>
        <li>
          <label>Dropoffs</label>
          <span>{{stats.frames[frame].players[index].currentDropoffs}}</span>
        </li>
        <li>
          <label>Collisions</label>
          <span>{{stats.frames[frame].players[index].currentCollisions}}</span>
        </li>
      </ul>
    </div>
  </div>
</div>
</template>

<script>
import Vue from 'vue'
import * as d3 from 'd3'
import PlayerHaliteChart from './PlayerHaliteChart.vue'
export default {
  name: 'PlayerDetail',
  props: ['players', 'replay', 'frame', 'stats', 'statistics', 'chartData'],
  data: function () {
    return {
      indexedPlayers: {}
    }
  },
  components: {
    PlayerHaliteChart
  },
  mounted() {
    for (const player of Object.values(this.players)) {
      this.indexedPlayers[player.player_index] = player
    }
  },
  computed: {
    playerInfo: function () {
      if (!this.stats) return null

      return this.stats.frames[this.frame].players
    },
    maxY() {
      if (!this.chartData || !this.chartData.length) {
        return 0;
      }
      return d3.max(this.chartData.map(player => d3.max(player, (d) => d.y)), (y) => y);
    },
    currentHalite() {
      let result = {
      }
      for(let index in this.statistics) {
        result[index] = this.frame > 0? this.replay.full_frames[this.frame - 1].energy[index]: this.replay.GAME_CONSTANTS.INITIAL_ENERGY
      }
      return result
    },
  },
  methods: {
    currentEfficiency(index) {
      if (!this.stats) return 0;
      const deposited = this.replay.GAME_CONSTANTS.INITIAL_ENERGY + this.stats.frames[this.frame].players[index].depositedHalite
      return this.currentHalite[index] / deposited
    },
    numberSep: function (number) {
      return number.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ',')
    },
    extraPlayerStats(index) {
      const playerStats = this.replay.game_statistics.player_statistics[index];
      return `Efficiency ${(playerStats.mining_efficiency * 100).toFixed(1)}%; total halite collected ${playerStats.total_mined}, inspiration bonus halite ${playerStats.total_bonus}, captured ships collected ${playerStats.total_mined_from_captured} halite`;
    },
  }
}
</script>
<style lang="scss" scoped>
  a {
    color: inherit;
  }
.player-cards-list{
  padding: 0;
  .card-player{
    margin: 0;
    padding: 6px 20px;
    padding-right: 0;
    color: #0C0C0C;
    display: flex;
 // justify-content: space-between;
    border-bottom: 1.2px solid rgba(8,27,83,.1);
  }
  .card-player-left{
    min-width: 140px;
  }
  .card-player-name{
    font-size: 14px;
    font-weight: 600;
    letter-spacing: 0.53px;
    line-height: 17px;
  }
  .player-current-halite{
    display: flex;
 // align-items: baseline;
    font-size: 28px;
    line-height: 34px;
    position: relative;
    .current-percent{
      margin-left: 5px;
      font-size: 16px;
      &.red {
        color: red;
      }
      &.green{
        color: green;
      }
    }
  }
  .stats-cube{
    width: 60px;
    margin-top: 10px;
  }
  .chart-box{
    width: 100%;
  }
  .player-stats-list{
    list-style: none;
    margin: 0;
    padding-left: 10px;
    li{
      display: flex;
      justify-content: space-between;
      line-height: 26px;
      padding-top: 10px;
      border-bottom: 1.2px solid rgba(8,27,83,.1);
      width: 170px;
      font-size: 14px;
      padding-right: 20px;
      &:last-child{
        border: none;
      }
      span{
        color: #003587;
      }
    }
  }

  @media (min-width:1200px) and (max-width: 1400px) {
    .chart-box {
      margin-left: -15px;
    }
    .player-stats-list {
      li {
        padding-right: 20px;
        width: auto;
        > span {
          margin-left: 15px;
        }
      }
    }
  }
}
</style>

<!--
     Local Variables:
     web-mode-script-padding: 0
     End:
     End: -->
