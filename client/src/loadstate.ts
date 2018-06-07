import {State} from './gameview';
import * as PIXI from 'pixi.js';



export abstract class LoadState extends State {

    private stage: PIXI.Container;
    private text: PIXI.Text;
    private nextState: State;

    constructor() {
        super();
        
        this.stage = new PIXI.Container();
        this.text = new PIXI.Text("Loading...", {fill: "#ffffff"});
        this.stage.addChild(this.text);
    }

    enter(prev: State, renderer: PIXI.SystemRenderer): void {
        this.text.anchor.x = 0.5;
        this.text.anchor.y = 0.5;
        this.text.x = renderer.width / 2;
        this.text.y = renderer.height / 2;

        let loader = new PIXI.loaders.Loader();
        this.load(loader);
        loader.once("complete", this.onLoaded.bind(this));
        loader.load();
    }

    private onLoaded(loader: PIXI.loaders.Loader, resources: any) {
        this.nextState = this.loaded(loader, resources);
    }

    protected abstract load(loader: PIXI.loaders.Loader): void;
    protected abstract loaded(loader: PIXI.loaders.Loader, resources: any): State;

    leave(next: State): void {

    }

    update(dt: number): State | null {
        return this.nextState;
    }

    render(dt: number): PIXI.Container | null {
        return this.stage;
    }
}