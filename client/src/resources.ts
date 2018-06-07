import * as PIXI from 'pixi.js';
import {State} from './gameview';
import {LoadState} from './loadstate';
import {GameState} from './gamestate';

export interface Resources {
    tiles: {
        wall: PIXI.Texture,
        ground: PIXI.Texture
    }
}


export class ResourceLoaderState extends LoadState {

    constructor() {
        super();
    }

    load(loader: PIXI.loaders.Loader): void {
        loader.add("atlas", "assets/tiles.json");
    }

    loaded(loader: PIXI.loaders.Loader, res: any): State {

        let tiles: any = res.atlas.textures;
        let tex = (name: string): PIXI.Texture => tiles[name];

        let resources: Resources = {
            tiles: {
                wall: tex("wall.png"),
                ground: tex("ground.png")
            }
        };

        return new GameState(resources);
    }

}