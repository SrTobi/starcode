import * as PIXI from 'pixi.js';
import {State} from './gameview';
import {Resources} from './resources';
import {World, WorldGenerator, Tile} from './world';
import {Entity} from './entity';
import {Creature} from './creature';
import {Key, Keys} from './keyboard';

const ScreenDimensionX = 10;
const ScreenDimensionY = 10;
const PlayerSpeed = 10;
 
class Controls {
    up = new Key(Keys.Up);
    down = new Key(Keys.Down);
    left = new Key(Keys.Left);
    right = new Key(Keys.Right);

    horizontalMovement() {
        let hm = 0;
        if(this.up.isDown()) {
            hm -= 1;
        }
        if(this.down.isDown()) {
            hm += 1;
        }
        return hm;
    }
    
    verticalMovement() {
        let vm = 0;
        if(this.left.isDown()) {
            vm -= 1;
        }
        if(this.right.isDown()) {
            vm += 1;
        }
        return vm;
    }
}


export class GameState extends State {

    private worldGenerator: WorldGenerator;
    private world: World;
    private entities: Entity[] = [];
    private stage = new PIXI.Container();
    private view = new PIXI.Container();
    private screenWidth: number;
    private screenHeight: number;
    private player: Creature;
    private ctrl = new Controls();

    constructor(private resources: Resources) {
        super();

        this.worldGenerator = new WorldGenerator(30, 30, resources);
    }

    enter(prev: State, renderer: PIXI.SystemRenderer): void {
        this.world = this.worldGenerator.buildWorld2();
        this.view.addChild(this.world);
        this.stage.addChild(this.view);

        this.player = this.addEntity(new Creature());
    }

    leave(next: State): void {
        this.view.removeChild(this.world);
    }



    addEntity<T extends Entity>(entity: T): T {
        this.entities.push(entity);
        this.view.addChild(entity);

        return entity;
    }

    update(dt: number): State | null {
        this.world.update(dt);

        this.entities.forEach(entity => entity.update(dt));

        // update player
        this.player.x += this.ctrl.verticalMovement() * PlayerSpeed * dt;
        this.player.y += this.ctrl.horizontalMovement() * PlayerSpeed * dt;

        if(this.ctrl.verticalMovement() == 0 && this.ctrl.horizontalMovement() == 0) {
            this.player.stand();
        }else{
            this.player.walk();
        }


        // update view
        this.view.x = -this.player.x;
        this.view.y = -this.player.y;
        this.world.setView(this.player.x, this.player.y, ScreenDimensionX, ScreenDimensionY);


        return null;
    }

    render(dt: number): PIXI.Container | null {
        return this.stage;
    }

    resized(renderer: PIXI.SystemRenderer): void {
        this.screenWidth = renderer.width;
        this.screenHeight = renderer.height;
        this.stage.x = this.screenWidth / 2;
        this.stage.y = this.screenHeight / 2;
        let ratio = Math.min(renderer.width / ScreenDimensionX, renderer.height / ScreenDimensionY);
        let scale = this.stage.scale;
        //scale.x = scale.y = ratio;
        scale.x = scale.y = ScreenDimensionX * 4;
        console.log("bla");
    }
}