import * as React from "react";
import * as PIXI from 'pixi.js';
import * as Utils from './utils';


export abstract class State {
    enter(prev: State, renderer: PIXI.SystemRenderer): void {}
    leave(next: State): void {}

    resized(renderer: PIXI.SystemRenderer): void {}

    abstract update(dt: number): State | null;
    abstract render(dt: number): PIXI.Container | null;
}

export class GameView extends React.Component<{firstState: State}, {}>{

    private renderer: PIXI.SystemRenderer;
    private lastUpdate: number;
    private currentState: State;
    private gameCanvas: HTMLElement;

    constructor(props: {firstState: State}) {
        super(props);
    }

    private run(state: State) {
        this.setGameState(state);
        this.lastUpdate = Date.now();
        this.onFrame();
    }

    private setGameState(nextState: State) {
        let prevState = this.currentState;
        if(prevState) {
            prevState.leave(nextState);
        }
        this.currentState = nextState;
        if(nextState) {
            nextState.enter(prevState, this.renderer);
            nextState.resized(this.renderer);
        }
    }

    private resized() {
        this.renderer.resize(this.gameCanvas.clientWidth, this.gameCanvas.clientWidth * 0.7);
        if(this.currentState) {
            this.currentState.resized(this.renderer);
        }
    }

    private onFrame() {
        let now = Date.now();
        let dt = (now - this.lastUpdate) / 1000;
        this.lastUpdate = now;

        if(this.currentState) {
            // render
            let stage = this.currentState.render(dt);
            if(stage) {
                this.renderer.render(stage);
            }

            // update
            let nextState = this.currentState.update(dt);
            if(nextState) {
                this.setGameState(nextState);
            }

            // reqest next frame
            requestAnimationFrame(this.onFrame.bind(this));
        }
    }

    componentDidMount() {
        this.renderer = PIXI.autoDetectRenderer(800, 600, {backgroundColor: 0x000000, antialias: true});

        this.gameCanvas.appendChild(this.renderer.view);
        this.resized();
        //this.renderer.resize(this.gameCanvas.clientWidth, this.gameCanvas.clientHeight);
        window.onresize = () => {this.resized();}

        console.log("componentDidMount");
        
        this.run(this.props.firstState);
    }

    render() {
        return (
                <div className="game-canvas-container" ref={(c) => this.gameCanvas = c}>
                </div>
            );
    }
}