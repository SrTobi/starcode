import * as PIXI from 'pixi.js';

export abstract class Entity extends PIXI.Container {
    abstract update(dt: number): void;
}