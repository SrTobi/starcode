import * as PIXI from 'pixi.js';
import {Entity} from './entity';

enum CreatureState {
    Standing,
    Walking
}

function near(first: PIXI.Point, sec: PIXI.Point, f = 0.05) {
    let w = (first.x - sec.x);
    let h = (first.y - sec.y);
    return w * w + h * h < f;
}

function add(first: PIXI.Point, sec: PIXI.Point) {
    return new PIXI.Point(first.x + sec.x, first.y + sec.y);
}

function sub(first: PIXI.Point, sec: PIXI.Point) {
    return new PIXI.Point(first.x - sec.x, first.y - sec.y);
}

function scale(p: PIXI.Point, s: number) {
    return new PIXI.Point(p.x * s, p.y * s);
}

const StandingTargetNear = 0.001;
const RightHandStandingTargets = [new PIXI.Point(5, 0), new PIXI.Point(4.3, 0)];
const LeftHandStandingTargets = [new PIXI.Point(-5, 0), new PIXI.Point(-4.3, 0)];

const WalkingTargetNear = 0.2;
const RightHandWalkingTargets = [new PIXI.Point(5, 4), new PIXI.Point(5, 0)];
const LeftHandWalkingTargets = [new PIXI.Point(-5, 0), new PIXI.Point(-5, 4)];

export class Creature extends Entity {
    
    private body = new PIXI.Graphics();
    private rightHand = new PIXI.Graphics();
    private rightHandTarget = new PIXI.Point(5, 0);
    private leftHand = new PIXI.Graphics();
    private leftHandTarget = new PIXI.Point(-5, 0);
    private state: CreatureState = CreatureState.Standing;

    constructor() {
        super();

        // body
        let g = this.body;
        g.lineStyle(1, 0x000000, 0.7);
        g.beginFill(0x010101);
        g.drawCircle(0, 0, 3);
        g.endFill();

        // right hand
        g = this.rightHand;
        g.lineStyle(1, 0x000000, 0.7);
        g.beginFill(0x010101);
        g.drawCircle(0, 0, 1);
        g.endFill();
        g.position = this.rightHandTarget;

        // left hand
        g = this.leftHand;
        g.lineStyle(1, 0x000000, 0.7);
        g.beginFill(0x010101);
        g.drawCircle(0, 0, 1);
        g.endFill();
        g.position = this.leftHandTarget;



        this.scale.x = 1/20;
        this.scale.y = 1/20;

        this.addChild(this.rightHand);
        this.addChild(this.leftHand);
        this.addChild(this.body);
    }

    private animateState = 0;
    private animateNear = 0.05;
    private stdAnimateStart(rightHandTargets: PIXI.Point[], leftHandTargets: PIXI.Point[], near = 0.05) {
        this.animateState = 0;
        this.rightHandTarget = rightHandTargets[0];
        this.leftHandTarget = leftHandTargets[0];
        this.animateNear = near;
    }

    private stdAnimate(rightHandTargets: PIXI.Point[], leftHandTargets: PIXI.Point[], dt: number) {
        if(near(this.leftHandTarget, this.leftHand.position, this.animateNear) && near(this.rightHand.position, this.rightHandTarget, this.animateNear)) {
            ++this.animateState;
            this.rightHandTarget = rightHandTargets[this.animateState % rightHandTargets.length];
            this.leftHandTarget = leftHandTargets[this.animateState % leftHandTargets.length];
        }
    }

    stand(): void {
        if(this.state != CreatureState.Standing) {
            this.stdAnimateStart(RightHandStandingTargets, LeftHandStandingTargets, StandingTargetNear);
        }
        this.state = CreatureState.Standing;
    }

    walk(): void {
        if(this.state != CreatureState.Walking) {
            this.stdAnimateStart(RightHandWalkingTargets, LeftHandWalkingTargets, WalkingTargetNear);
        }
        this.state = CreatureState.Walking;
    }

    update(dt: number): void {
        if(this.state == CreatureState.Walking) {
            this.stdAnimate(RightHandWalkingTargets, LeftHandWalkingTargets, dt);
        }

        if(this.state == CreatureState.Standing) {
            this.stdAnimate(RightHandStandingTargets, LeftHandStandingTargets, dt);
        }
        
        this.moveHand(this.rightHand, this.rightHandTarget, dt);
        this.moveHand(this.leftHand, this.leftHandTarget, dt);
    }

    private moveHand(hand: PIXI.Container, target: PIXI.Point, dt: number) {
        let diff = sub(target, hand.position);
        let mov = scale(diff, 0.5 * dt * 10);
        hand.position = add(hand.position, mov);
    }
}