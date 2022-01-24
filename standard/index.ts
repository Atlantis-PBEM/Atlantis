import Color from 'color'
import Rainbow from 'rainbowvis.js'
import data from './map.json'
import hexdata from './hexmap.json'
import { Point, DoubledCoord, Layout } from './hex'

const canvas = document.getElementById('map') as HTMLCanvasElement;
const ctx = canvas.getContext('2d');

interface Range {
    min: number
    max: number
    r: number
}

function range(value: number, r?: Range): Range {
    if (!r) {
        return {
            min: value,
            max: value,
            r: 0
        }
    }

    const min = Math.min(r.min, value)
    const max = Math.max(r.max, value)

    return {
        min,
        max,
        r: Math.abs(max - min)
    }
}

const STATS = {
    elevation: null as Range,
    temperature: null as Range,
    saturation: null as Range,
    evoparation: null as Range,
    rainfall: null as Range
}

for (const item of data) {
    STATS.elevation = range(item.elevation, STATS.elevation)
    STATS.temperature = range(item.temperature, STATS.temperature)
    STATS.saturation = range(item.saturation, STATS.saturation)
    STATS.evoparation = range(item.evoparation, STATS.evoparation)
    STATS.rainfall = range(item.rainfall, STATS.rainfall)
}

console.log(STATS)

const water = new Rainbow();
water.setNumberRange(0, Math.abs(STATS.elevation.min))
water.setSpectrum('#5DADE2', Color('#5DADE2').darken(0.5).rgb().hex());

const land = new Rainbow();
land.setNumberRange(0, STATS.elevation.max)
land.setSpectrum('#CACFD2', Color('#CACFD2').darken(0.5).rgb().hex());

const temperature = new Rainbow();
temperature.setNumberRange(STATS.temperature.min, STATS.temperature.max)
temperature.setSpectrum('blue', 'green', 'red');

const evoparation = new Rainbow();
evoparation.setNumberRange(0, STATS.evoparation.max)
evoparation.setSpectrum('white', '#5DADE2');

const rainfall = new Rainbow();
rainfall.setNumberRange(0, STATS.rainfall.max)
rainfall.setSpectrum('white', '#5DADE2');

type Cell = typeof data[0]
type Region = typeof hexdata[0];

interface ColorFunction {
    (item: Cell): string
}

const taiga = Color('#4ba666');
const jungle = Color('#177832');
const grassland = Color('#91e069');
const dessert = Color('#f0d735');

const BIOME = {
    0: 'red', // B_UNKNOWN
    1: '#f8f8f8', // B_TUNDRA
    2: '#bbbbbb', // B_MOUNTAINS
    3: '#828c51', // B_SWAMP
    4: taiga.rgb().hex(), // B_FOREST
    5: grassland.rgb().hex(), // B_PLAINS
    6: jungle.rgb().hex(), // B_JUNGLE
    7: dessert.rgb().hex(), // B_DESERT
    8: '#5dade2', // B_WATER
}

const TERRAIN = {
    7: '#f8f8f8', // B_TUNDRA
    3: '#bbbbbb', // B_MOUNTAINS
    4: '#828c51', // B_SWAMP
    2: taiga.rgb().hex(), // B_FOREST
    1: grassland.rgb().hex(), // B_PLAINS
    5: jungle.rgb().hex(), // B_JUNGLE
    6: dessert.rgb().hex(), // B_DESERT
    0: '#5dade2', // B_WATER
}

const COLOR = {
    elevation: (cell: Cell) => {
        if (cell.biome === 2) {
            return 'black'
        }

        return `#${(cell.elevation > 0 ? land : water).colorAt(Math.abs(cell.elevation))}`
    },

    temperature: (cell: Cell) => {
        return `#${temperature.colorAt(cell.temperature)}`
    },

    evoparation: (cell: Cell) => {
        return `#${evoparation.colorAt(cell.evoparation)}`
    },

    rainfall: (cell: Cell) => {
        return `#${rainfall.colorAt(cell.rainfall)}`
    },

    biome: (cell: Cell) => {
        return BIOME[cell.biome]
    }
}

const CELL_SZ = 4
const MAP_W = 128
const MAP_H = 128
const MAP_GAP = 32

function drawMap(col: number, row: number, color: ColorFunction, label?: (cell: Cell) => string) {
    const offsetX = col * (MAP_W * CELL_SZ + MAP_GAP) + 50;
    const offsetY = row * (MAP_H * CELL_SZ + MAP_GAP) + 50;

    for (const item of data) {
        ctx.fillStyle = color(item);

        const x = item.x * CELL_SZ + offsetX
        const y = item.y * CELL_SZ + offsetY

        ctx.fillRect(x, y, CELL_SZ, CELL_SZ)
    }

    for (const item of data) {
        const x = item.x * CELL_SZ + offsetX
        const y = item.y * CELL_SZ + offsetY

        if (label) {
            const l = label(item);
            if (l) {
                ctx.save();

                ctx.fillStyle = 'white'
                ctx.fillRect(x + 1, y + 1, 3, 3)

                ctx.fillStyle = 'black'
                ctx.fillRect(x + 2, y + 2, 1, 1)

                ctx.font = 'bold 10pt monospace'
                ctx.textBaseline = 'middle'
                ctx.textAlign = 'left'
                ctx.shadowOffsetX = 1;
                ctx.shadowOffsetY = 1;
                ctx.shadowColor = 'white';
                ctx.shadowBlur = 1;
                ctx.fillText(l, x + CELL_SZ * 2, y)

                ctx.restore();
            }
        }
    }
}

function drawHexMap(col: number, row: number) {
    const offsetX = col * (MAP_W * CELL_SZ + MAP_GAP) + 50;
    const offsetY = row * (MAP_H * CELL_SZ + MAP_GAP) + 50;

    const layout = new Layout(Layout.flat, new Point(CELL_SZ * 2, CELL_SZ * 2.25), new Point(offsetX, offsetY));

    for (const item of hexdata) {
        const p = new DoubledCoord(item.x, item.y).qdoubledToCube();
        const points = layout.polygonCorners(p);

        ctx.fillStyle = TERRAIN[item.type];
        ctx.beginPath();
        let first = true;
        for (const n of points) {
            if (first) ctx.moveTo(n.x, n.y); else ctx.lineTo(n.x, n.y);

            first = false;
        }
        ctx.closePath();
        ctx.fill();
    }
}

let minElevation = false
let maxElevation = false
drawMap(0, 0, COLOR.elevation, cell => {
    if (!minElevation && cell.elevation === STATS.elevation.min) {
        minElevation = true
        return `${cell.elevation}m`
    }

    if (!maxElevation && cell.elevation === STATS.elevation.max) {
        maxElevation = true
        return `${cell.elevation}m`
    }
})

let minTemp = false
let maxTemp = false
let zeroTemp = false
drawMap(1, 0, COLOR.temperature, cell => {
    if (!minTemp && cell.temperature === STATS.temperature.min) {
        minTemp = true
        return `${cell.temperature}°C`
    }

    if (!maxTemp && cell.temperature === STATS.temperature.max) {
        maxTemp = true
        return `${cell.temperature}°C`
    }

    if (!zeroTemp && cell.temperature === 0) {
        zeroTemp = true
        return `${cell.temperature}°C`
    }
})

drawMap(2, 0, COLOR.biome)
drawMap(0, 1, COLOR.evoparation)
drawMap(1, 1, COLOR.rainfall)

drawHexMap(2, 1)