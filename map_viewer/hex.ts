export class Point
{
    constructor (public x:number, public y:number) {}
}

export class Hex
{
    constructor (public q:number, public r:number, public s:number) {
        if (Math.round(q + r + s) !== 0) throw "q + r + s must be 0";
    }

    public add(b:Hex):Hex
    {
        return new Hex(this.q + b.q, this.r + b.r, this.s + b.s);
    }


    public subtract(b:Hex):Hex
    {
        return new Hex(this.q - b.q, this.r - b.r, this.s - b.s);
    }


    public scale(k:number):Hex
    {
        return new Hex(this.q * k, this.r * k, this.s * k);
    }


    public rotateLeft():Hex
    {
        return new Hex(-this.s, -this.q, -this.r);
    }


    public rotateRight():Hex
    {
        return new Hex(-this.r, -this.s, -this.q);
    }

    public static directions:Hex[] = [new Hex(1, 0, -1), new Hex(1, -1, 0), new Hex(0, -1, 1), new Hex(-1, 0, 1), new Hex(-1, 1, 0), new Hex(0, 1, -1)];

    public static direction(direction:number):Hex
    {
        return Hex.directions[direction];
    }


    public neighbor(direction:number):Hex
    {
        return this.add(Hex.direction(direction));
    }

    public static diagonals:Hex[] = [new Hex(2, -1, -1), new Hex(1, -2, 1), new Hex(-1, -1, 2), new Hex(-2, 1, 1), new Hex(-1, 2, -1), new Hex(1, 1, -2)];

    public diagonalNeighbor(direction:number):Hex
    {
        return this.add(Hex.diagonals[direction]);
    }


    public len():number
    {
        return (Math.abs(this.q) + Math.abs(this.r) + Math.abs(this.s)) / 2;
    }


    public distance(b:Hex):number
    {
        return this.subtract(b).len();
    }


    public round():Hex
    {
        var qi:number = Math.round(this.q);
        var ri:number = Math.round(this.r);
        var si:number = Math.round(this.s);
        var q_diff:number = Math.abs(qi - this.q);
        var r_diff:number = Math.abs(ri - this.r);
        var s_diff:number = Math.abs(si - this.s);
        if (q_diff > r_diff && q_diff > s_diff)
        {
            qi = -ri - si;
        }
        else
            if (r_diff > s_diff)
            {
                ri = -qi - si;
            }
            else
            {
                si = -qi - ri;
            }
        return new Hex(qi, ri, si);
    }


    public lerp(b:Hex, t:number):Hex
    {
        return new Hex(this.q * (1.0 - t) + b.q * t, this.r * (1.0 - t) + b.r * t, this.s * (1.0 - t) + b.s * t);
    }


    public linedraw(b:Hex):Hex[]
    {
        var N:number = this.distance(b);
        var a_nudge:Hex = new Hex(this.q + 1e-06, this.r + 1e-06, this.s - 2e-06);
        var b_nudge:Hex = new Hex(b.q + 1e-06, b.r + 1e-06, b.s - 2e-06);
        var results:Hex[] = [];
        var step:number = 1.0 / Math.max(N, 1);
        for (var i = 0; i <= N; i++)
        {
            results.push(a_nudge.lerp(b_nudge, step * i).round());
        }
        return results;
    }

}

export class OffsetCoord
{
    constructor (public col:number, public row:number) {}
    public static EVEN:number = 1;
    public static ODD:number = -1;

    public static qoffsetFromCube(offset:number, h:Hex):OffsetCoord
    {
        var col:number = h.q;
        var row:number = h.r + (h.q + offset * (h.q & 1)) / 2;
        if (offset !== OffsetCoord.EVEN && offset !== OffsetCoord.ODD)
        {
            throw "offset must be EVEN (+1) or ODD (-1)";
        }
        return new OffsetCoord(col, row);
    }


    public static qoffsetToCube(offset:number, h:OffsetCoord):Hex
    {
        var q:number = h.col;
        var r:number = h.row - (h.col + offset * (h.col & 1)) / 2;
        var s:number = -q - r;
        if (offset !== OffsetCoord.EVEN && offset !== OffsetCoord.ODD)
        {
            throw "offset must be EVEN (+1) or ODD (-1)";
        }
        return new Hex(q, r, s);
    }


    public static roffsetFromCube(offset:number, h:Hex):OffsetCoord
    {
        var col:number = h.q + (h.r + offset * (h.r & 1)) / 2;
        var row:number = h.r;
        if (offset !== OffsetCoord.EVEN && offset !== OffsetCoord.ODD)
        {
            throw "offset must be EVEN (+1) or ODD (-1)";
        }
        return new OffsetCoord(col, row);
    }


    public static roffsetToCube(offset:number, h:OffsetCoord):Hex
    {
        var q:number = h.col - (h.row + offset * (h.row & 1)) / 2;
        var r:number = h.row;
        var s:number = -q - r;
        if (offset !== OffsetCoord.EVEN && offset !== OffsetCoord.ODD)
        {
            throw "offset must be EVEN (+1) or ODD (-1)";
        }
        return new Hex(q, r, s);
    }

}

export class DoubledCoord
{
    constructor (public col:number, public row:number) {}

    public static qdoubledFromCube(h:Hex):DoubledCoord
    {
        var col:number = h.q;
        var row:number = 2 * h.r + h.q;
        return new DoubledCoord(col, row);
    }


    public qdoubledToCube():Hex
    {
        var q:number = this.col;
        var r:number = (this.row - this.col) / 2;
        var s:number = -q - r;
        return new Hex(q, r, s);
    }


    public static rdoubledFromCube(h:Hex):DoubledCoord
    {
        var col:number = 2 * h.q + h.r;
        var row:number = h.r;
        return new DoubledCoord(col, row);
    }


    public rdoubledToCube():Hex
    {
        var q:number = (this.col - this.row) / 2;
        var r:number = this.row;
        var s:number = -q - r;
        return new Hex(q, r, s);
    }

}

export class Orientation
{
    constructor (public f0:number, public f1:number, public f2:number, public f3:number, public b0:number, public b1:number, public b2:number, public b3:number, public start_angle:number) {}
}

export class Layout
{
    constructor (public orientation:Orientation, public size:Point, public origin:Point) {}
    public static pointy:Orientation = new Orientation(Math.sqrt(3.0), Math.sqrt(3.0) / 2.0, 0.0, 3.0 / 2.0, Math.sqrt(3.0) / 3.0, -1.0 / 3.0, 0.0, 2.0 / 3.0, 0.5);
    public static flat:Orientation = new Orientation(3.0 / 2.0, 0.0, Math.sqrt(3.0) / 2.0, Math.sqrt(3.0), 2.0 / 3.0, 0.0, -1.0 / 3.0, Math.sqrt(3.0) / 3.0, 0.0);

    public hexToPixel(h:Hex):Point
    {
        var M:Orientation = this.orientation;
        var size:Point = this.size;
        var origin:Point = this.origin;
        var x:number = (M.f0 * h.q + M.f1 * h.r) * size.x;
        var y:number = (M.f2 * h.q + M.f3 * h.r) * size.y;
        return new Point(x + origin.x, y + origin.y);
    }


    public pixelToHex(p:Point):Hex
    {
        var M:Orientation = this.orientation;
        var size:Point = this.size;
        var origin:Point = this.origin;
        var pt:Point = new Point((p.x - origin.x) / size.x, (p.y - origin.y) / size.y);
        var q:number = M.b0 * pt.x + M.b1 * pt.y;
        var r:number = M.b2 * pt.x + M.b3 * pt.y;
        return new Hex(q, r, -q - r);
    }


    public hexCornerOffset(corner:number):Point
    {
        var M:Orientation = this.orientation;
        var size:Point = this.size;
        var angle:number = 2.0 * Math.PI * (M.start_angle - corner) / 6.0;
        return new Point(size.x * Math.cos(angle), size.y * Math.sin(angle));
    }


    public polygonCorners(h:Hex):Point[]
    {
        var corners:Point[] = [];
        var center:Point = this.hexToPixel(h);
        for (var i = 0; i < 6; i++)
        {
            var offset:Point = this.hexCornerOffset(i);
            corners.push(new Point(center.x + offset.x, center.y + offset.y));
        }
        return corners;
    }

}
