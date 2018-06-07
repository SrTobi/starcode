import * as React from "react";
import * as PIXI from 'pixi.js';
import {GameView} from './gameview';
import * as Utils from './utils';
import {ResourceLoaderState} from './resources';


interface AppState {
    gameView: JSX.Element | null;
}

export class App extends React.Component<{}, AppState>{

    public state: AppState = { gameView: null };

    private login(token: String) {
        alert(token);
        this.setState({
            gameView: (<GameView  firstState={new ResourceLoaderState()} />)
        });
    }

    private logout() {
        this.setState({
            gameView: null
        });
    }

    render() {
        var tokenInput: HTMLInputElement;
        const view = this.state.gameView || (
            <div className="login-view">
                <p>Enter Token:</p>
                <form>
                    <input type="input" ref={inp => tokenInput = inp}/>
                    <input type="submit" value="Login" onClick={() => this.login(tokenInput.value)}/>
                </form>
            </div>
        );
        return (
                <div>
                    <a href="https://github.com/srtobi/lambda">
                        <img
                            style={{position: "absolute", top: 0, right: 0, border: 0}}
                            src="https://camo.githubusercontent.com/a6677b08c955af8400f44c6298f40e7d19cc5b2d/68747470733a2f2f73332e616d617a6f6e6177732e636f6d2f6769746875622f726962626f6e732f666f726b6d655f72696768745f677261795f3664366436642e706e67"
                            alt="Fork me on GitHub"
                            data-canonical-src="https://s3.amazonaws.com/github/ribbons/forkme_right_gray_6d6d6d.png" />
                    </a>
                    <div className="main-container">
                        <div className="page-header-container">
                            <button type="button" className="btn btn-default logout-button" onClick={() => this.logout()}>
                                <span className="glyphicon glyphicon-off"></span>
                            </button>
                            <h1>StarCode</h1>
                        </div>
                        <div className="content-container">
                            {view}
                        </div>
                    </div>
                </div>
            );
    }
}