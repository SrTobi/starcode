



function setProp(obj: any, name: string, value: any) {
    obj[name] = value;
}

let p: {
    name?: string,
    age?: number
} = {};

setProp(p, "name", "test");



