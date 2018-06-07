import * as React from "react";
import * as ReactDOM from "react-dom";
import * as PIXI from 'pixi.js';
import {App} from './app';
import {ResourceLoaderState} from './resources';


import "jquery";
import "bootstrap/dist/js/bootstrap";

import "bootstrap/dist/css/bootstrap.min.css";
import "./css/style.css";


const app = <App />;

var target = document.createElement("div");
ReactDOM.render(app, target);
document.body.appendChild(target);
