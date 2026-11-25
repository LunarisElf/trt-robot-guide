// เช็คตัวจำลองใน robot-guide.html: รัน  node docs/check-sim.js
// ภารกิจตัวอย่างบนสนามฝึกต้องดันกระป๋องครบ และจอดทั้งคันในกล่อง 20x20
const fs = require('fs'), path = require('path');
const html = fs.readFileSync(path.join(__dirname, 'robot-guide.html'), 'utf8');
const code = html.match(/<script>([\s\S]*?)<\/script>/)[1];   // สคริปต์แรก = SIM
const SIM = new Function('module', code + '; return SIM;')({});

const R = SIM.runner(SIM.practice(), { x: 11, y: 62, th: -Math.PI / 2 }, SIM.MISSION_P);
while (R.status === 'running' && R.run.r.t < 120000) R.advance(50);
const { r, field } = R.run;
const moved = field.cans.filter(c => Math.hypot(c.x - c.x0, c.y - c.y0) > 2).length;
const [bx0, by0, bx1, by1] = field.box, c = Math.cos(r.th), s = Math.sin(r.th);
const inBox = [[SIM.FRONT, SIM.HALF_W], [SIM.FRONT, -SIM.HALF_W], [-SIM.REAR, SIM.HALF_W], [-SIM.REAR, -SIM.HALF_W]]
  .every(([f, l]) => { const x = r.x + c * f + s * l, y = r.y + s * f - c * l; return x >= bx0 && x <= bx1 && y >= by0 && y <= by1; });

console.assert(R.status === 'done', 'mission status ' + R.status);
console.assert(moved === field.cans.length, `cans ${moved}/${field.cans.length}`);
console.assert(inBox, 'robot not fully inside the start box');
console.log(`status ${R.status}, cans ${moved}/${field.cans.length}, in box ${inBox}, time ${(r.t / 1000).toFixed(1)} s`);
process.exitCode = R.status === 'done' && moved === field.cans.length && inBox ? 0 : 1;
