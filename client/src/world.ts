import * as PIXI from 'pixi.js';
import * as Utils from './utils';
import {Resources} from './resources';

export abstract class Tile extends PIXI.Container {
    abstract isWalkable(): boolean;
    abstract isStatic(): boolean;

    update(dt: number): void {}
}

class Wall extends Tile {

    constructor(img: PIXI.Sprite) {
        super();
        this.addChild(img);
    }

    isWalkable(): boolean {
        return false;
    }

    isStatic(): boolean {
        return true;
    }
}

class Ground extends Tile {
    constructor(img: PIXI.Sprite) {
        super();
        this.addChild(img);
    }

    isWalkable(): boolean {
        return true;
    }

    isStatic(): boolean {
        return true;
    }
}

export class World extends PIXI.Container {

    private dynamicTiles: Tile[] = [];

    constructor(
        public width: number,
        public height: number,
        private tiles: Tile[][]) {

        super();

        for(let x = 0; x < width; ++x) {
            for(let y = 0; y < height; ++y) {
                const tile = this.at(x, y);
                this.addChild(tile);

                if(!tile.isStatic()) {
                    this.dynamicTiles.push(tile);
                }
            }
        }
    }

    setView(x: number, y: number, width: number, height: number) {
        
    }

    at(x: number, y: number): Tile {
        return this.tiles[x][y];
    }

    update(dt: number): void {
        this.dynamicTiles.forEach(tile => tile.update(dt));
    }
}

class Interval {
    private _min: number;
    private _max: number;

    constructor(begin: number, end: number) {
        this._min = Math.min(begin, end);
        this._max = Math.max(begin, end);
    }

    min(): number {
        return this._min;
    }

    max(): number {
        return this._max;
    }

    size(): number {
        return this.max() - this.min() + 1; // max is inclusive!
    }

    intersects(other: Interval): boolean {
        return Math.min(this.max(), other.max()) >= Math.max(this.min(), other.min());
    }

    intersect(other: Interval): Interval {
        if(!this.intersects(other)) {
            throw "Intervals do not intersect!";
        }
        return new Interval(Math.max(this.min(), other.min()), Math.min(this.max(), other.max()));
    }

    hull(other: Interval): Interval {
        return new Interval(Math.min(this.min(), other.min()), Math.max(this.max(), other.max()));
    }

    rand(): number {
        return this.min() + Math.floor(this.size() * Math.random());
    }
}

abstract class RoomType {
    abstract width(): Interval;
    abstract height(): Interval;

    abstract topDoors(): Interval[];
    abstract bottomDoors(): Interval[];
    abstract leftDoors(): Interval[];
    abstract rightDoors(): Interval[];
}

interface Connection {
    first: Sector,
    second: Sector, 
    gate: {
        x: number,
        y: number
    }
}

class Sector {
    topNeighbours: Sector[] = [];
    bottomNeighbours: Sector[] = [];
    leftNeighbours: Sector[] = [];
    rightNeighbours: Sector[] = [];
    connections: Connection[] = [];
    visited = false;

    constructor(public horizontalBounds: Interval, public verticalBounds: Interval) {

    }

    width(): number {
        return this.horizontalBounds.size();
    }

    height(): number {
        return this.verticalBounds.size();
    }

    canSplitHorizontally(minSectorWidth: number): boolean {
        return this.width() >= minSectorWidth * 2 + 1;
    }
    
    canSplitVertically(minSectorHeight: number): boolean {
        return this.height() >= minSectorHeight * 2 + 1;
    }

    canSplit(minSectorWidth: number, minSectorHeight: number): boolean {
        return this.canSplitHorizontally(minSectorWidth) || this.canSplitVertically(minSectorHeight);
    }

    split(minSectorWidth: number, minSectorHeight: number): Sector[] {
        if(!this.canSplit(minSectorWidth, minSectorHeight)) {
            return [this];
        }
        
        if(this.canSplitHorizontally(minSectorWidth) && Math.random() > 0.5 || !this.canSplitVertically(minSectorHeight)) {
            return this.splitHorizontally(minSectorWidth);
        }else{
            return this.splitVertically(minSectorHeight);
        }
    }

    public splitHorizontally(minSectorWidth: number): Sector[] {
        let space = this.width() - 1; // 1 tile wall
        let sizeBounds = new Interval(minSectorWidth, space - minSectorWidth);
        let rightSize = sizeBounds.rand();
        let leftSize = space - rightSize;

        let hMin = this.horizontalBounds.min();
        let hMax = this.horizontalBounds.max();
        let left = new Sector(new Interval(hMin, hMin + leftSize - 1), this.verticalBounds);
        let right = new Sector(new Interval(hMax - rightSize + 1, hMax), this.verticalBounds);

        // neighbour check
        let neighboursFrom = (ref: Sector, list: Sector[]) => list.filter(n => ref.horizontalBounds.intersects(n.horizontalBounds));

        // adjust neighbours
        left.rightNeighbours = [right];
        left.leftNeighbours = this.leftNeighbours.slice(0);
        left.topNeighbours = neighboursFrom(left, this.topNeighbours);
        left.bottomNeighbours = neighboursFrom(left, this.bottomNeighbours);

        right.leftNeighbours = [left];
        right.rightNeighbours = this.rightNeighbours.slice(0);
        right.topNeighbours = neighboursFrom(right, this.topNeighbours);
        right.bottomNeighbours = neighboursFrom(right, this.bottomNeighbours);
        

        // update neighbours
        this.leftNeighbours.forEach(n => n.updateNeighbours(this, [left], s => s.rightNeighbours, s => s.verticalBounds))
        this.rightNeighbours.forEach(n => n.updateNeighbours(this, [right], s => s.leftNeighbours, s => s.verticalBounds))
        this.topNeighbours.forEach(n => n.updateNeighbours(this, [left, right], s => s.bottomNeighbours, s => s.horizontalBounds))
        this.bottomNeighbours.forEach(n => n.updateNeighbours(this, [left, right], s => s.topNeighbours, s => s.horizontalBounds))

        return [left, right];
    }

    public splitVertically(minSectorHeight: number): Sector[] {
        let space = this.height() - 1; // 1 tile wall
        let sizeBounds = new Interval(minSectorHeight, space - minSectorHeight);
        let topSize = sizeBounds.rand();
        let bottomSize = space - topSize;

        let vMin = this.verticalBounds.min();
        let vMax = this.verticalBounds.max();
        let top = new Sector(this.horizontalBounds, new Interval(vMin, vMin + topSize - 1));
        let bottom = new Sector(this.horizontalBounds, new Interval(vMax - bottomSize + 1, vMax));

        // neighbour check
        let neighboursFrom = (ref: Sector, list: Sector[]) => list.filter(n => ref.verticalBounds.intersects(n.verticalBounds));

        // adjust neighbours
        top.bottomNeighbours = [bottom];
        top.topNeighbours = this.topNeighbours.slice(0);
        top.rightNeighbours = neighboursFrom(top, this.rightNeighbours);
        top.leftNeighbours = neighboursFrom(top, this.leftNeighbours);
        
        bottom.topNeighbours = [top];
        bottom.bottomNeighbours = this.bottomNeighbours.slice(0);
        bottom.rightNeighbours = neighboursFrom(bottom, this.rightNeighbours);
        bottom.leftNeighbours = neighboursFrom(bottom, this.leftNeighbours);

        // update neighbours
        this.topNeighbours.forEach(n => n.updateNeighbours(this, [top], s => s.bottomNeighbours, s => s.horizontalBounds))
        this.bottomNeighbours.forEach(n => n.updateNeighbours(this, [bottom], s => s.topNeighbours, s => s.horizontalBounds))
        this.leftNeighbours.forEach(n => n.updateNeighbours(this, [top, bottom], s => s.rightNeighbours, s => s.verticalBounds))
        this.rightNeighbours.forEach(n => n.updateNeighbours(this, [top, bottom], s => s.leftNeighbours, s => s.verticalBounds))
        
        return [top, bottom];
    }

    private updateNeighbours(old: Sector, newSectors: Sector[], side: (s: Sector) => Sector[], bounds: (s: Sector) => Interval) {
        let sideNeighbours = side(this);

        let idx = sideNeighbours.indexOf(old);
        let b = bounds(this);
        let adjacentSectors = newSectors.filter(sec => b.intersects(bounds(sec)));

        sideNeighbours.splice(idx, 1,...adjacentSectors);
    }

    public neighbours(): Sector[] {
        return this.topNeighbours.concat(this.bottomNeighbours, this.leftNeighbours, this.rightNeighbours);
    }

    public isNeighbour(other: Sector): boolean {
        return this.neighbours().indexOf(other) >= 0;
    }

    public connect(other: Sector) {
        let connection: Connection = {
            first: this,
            second: other,
            gate: {
                x: 0,
                y: 0
            }
        };

        if(this.verticalBounds.intersects(other.verticalBounds)) {
            // horizontal gate
            let gateInterval = this.verticalBounds.intersect(other.verticalBounds);

            connection.gate.x = Math.min(this.horizontalBounds.max(), other.horizontalBounds.max()) + 1;
            connection.gate.y = gateInterval.rand();
        } else {
            // vertical gate
            let gateInterval = this.horizontalBounds.intersect(other.horizontalBounds);

            connection.gate.x = gateInterval.rand();
            connection.gate.y = Math.min(this.verticalBounds.max(), other.verticalBounds.max()) + 1;
        }

        this.connections.push(connection);
        other.connections.push(connection);
    }
}

export class WorldGenerator {

    roomTypes: RoomType[];

    constructor(
        private width: number,
        private height: number,
        private resources: Resources) {

    }

    buildWorld(): World {
        let tiles: Tile[][] = [];
        let groundTex = this.resources.tiles.ground;
        let wallTex = this.resources.tiles.wall;

        let mkSprite = (tex: PIXI.Texture) => {
            let sprite = new PIXI.Sprite(tex);
            sprite.scale.x = 1/sprite.width;
            sprite.scale.y = 1/sprite.height;
            return sprite;
        }

        for(let x = 0; x < this.width; ++x) {
            tiles[x] = [];
            for(let y = 0; y < this.height; ++y) {
                let tile: Tile;

                if(x == 0 || x == this.width - 1 || y == 0 || y == this.height - 1) {
                    tile = new Wall(mkSprite(wallTex));    
                } else {
                    tile = new Ground(mkSprite(groundTex));
                }
                tile.x = x;
                tile.y = y;

                tiles[x][y] = tile;
            }
        }

        return new World(this.width, this.height, tiles);
    }

    buildWorld2(): World {

        const minSectorWidth = 2;
        const minSectorHeight = 2;
        // create a single sector
        let sectors = [new Sector(new Interval(1, this.width - 2), new Interval(1, this.height - 2))];

        let maxSplitTrys = 100;

        while(maxSplitTrys-- > 0) {
            // split a random sector 
            let secIdx = Utils.randIndex(sectors);
            let sector = sectors[secIdx];

            if(sector.canSplit(minSectorWidth, minSectorHeight)) {
                sectors.splice(secIdx, 1);
                let newSecs = sector.split(minSectorWidth, minSectorHeight);
                sectors.push(...newSecs);
            }
        }


        let tiles: Tile[][] = [];
        let groundTex = this.resources.tiles.ground;
        let wallTex = this.resources.tiles.wall;

        let mkSprite = (tex: PIXI.Texture) => {
            let sprite = new PIXI.Sprite(tex);
            sprite.scale.x = 1/sprite.width;
            sprite.scale.y = 1/sprite.height;
            return sprite;
        }

        // make everything a wall
        for(let x = 0; x < this.width; ++x) {
            tiles[x] = [];
            for(let y = 0; y < this.height; ++y) {
                let tile = new Wall(mkSprite(wallTex));
                tile.x = x;
                tile.y = y;

                tiles[x][y] = tile;
            }
        }

        // remove sector bodys
        for(let sector of sectors) {
            for(let x = sector.horizontalBounds.min(); x <= sector.horizontalBounds.max(); ++x) {
                for(let y = sector.verticalBounds.min(); y <= sector.verticalBounds.max(); ++y) {
                    let tile = new Ground(mkSprite(groundTex));
                    tile.x = x;
                    tile.y = y;

                    tiles[x][y] = tile;
                }
            }
        }

        // find route through sectors
        type StackType = [Sector, Sector | undefined];
        let sectorStack: [StackType] = [[Utils.randElement(sectors), undefined]];
        
        while(sectorStack.length > 0) {
            let cur = sectorStack.pop();

            if(cur && !cur[0].visited) {
                let sector = cur[0];
                sector.visited = true;

                // create connecton
                let origin = cur[1];
                if(origin) {
                    origin.connect(sector);
                }
                let neighbours = sector.neighbours();
                Utils.shuffle(neighbours);

                let nexts = neighbours.map((s):StackType => [s, sector]);

                sectorStack.push(...nexts);
            }
        }



        // add gates
        for(let cur of sectors) {

            for(let con of cur.connections) {
                let x = con.gate.x;
                let y = con.gate.y;

                tiles[x][y].alpha = 0.5;
            }
        }


        return new World(this.width, this.height, tiles);
    }
}