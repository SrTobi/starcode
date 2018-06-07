
export class Key {

    public press: () => void;
    public release: () => void;
    private _isPressed: boolean = false;

    constructor(private keycode: number) {
        window.addEventListener("keydown", this.handleDown.bind(this));
        window.addEventListener("keyup", this.handleUp.bind(this));
    }

    public isDown(): boolean {
        return this._isPressed;
    }

    public isUp(): boolean {
        return !this._isPressed;
    }

    private handleDown(event: KeyboardEvent) {
        if(event.keyCode == this.keycode) {
            if(this.isUp() && this.press) {
                this.press();
            }
            this._isPressed = true;
            event.preventDefault();
        }
    }

    private handleUp(event: KeyboardEvent) {
        if(event.keyCode == this.keycode) {
            if(this.isDown() && this.release) {
                this.release();
            }
            this._isPressed = false;
            event.preventDefault();
        }
    }
}

export enum Keys {
    A = 65,
    B,
    C,
    D,
    E,
    F,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    _0 = 48,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    Backspace = 8,
    Tab = 9,
    Enter = 13,
    Shift = 16,
    Ctrl = 17,
    CapsLock = 20,
    Esc = 27,
    Space = 32,
    PageUp = 33,
    PageDown = 34,
    End = 35,
    Home = 36,
    Left = 37,
    Up = 38,
    Right = 39,
    Down = 40,
    Insert = 45,
    Delete = 46
}