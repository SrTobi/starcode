

export function radToDeg(rad: number) {
    return rad * (180/Math.PI);
}

export function degToRad(deg: number) {
    return deg * (Math.PI/180);
}

export function randIndex<T>(array: T[]): number {
    return Math.floor(Math.random() * array.length);
}

export function randElement<T>(array: T[]): T {
    return array[Math.floor(Math.random() * array.length)];
}

export function shuffle<T>(array: T[]): T[] {
    for (let i = array.length; i > 0; i--) {
        let j = Math.floor(Math.random() * i);
        let tmp = array[i - 1];
        array[i - 1] = array[j];
        array[j] = tmp;
    }

    return array;
}