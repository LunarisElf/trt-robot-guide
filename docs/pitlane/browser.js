/* Command Browser: แสดงทีละหน้า ค้นหาได้ ไม่แตะ sim เดิม */
(() => {
  const $ = (s, e = document) => e.querySelector(s), $$ = (s, e = document) => [...e.querySelectorAll(s)];
  const root = document.documentElement, rail = $('.rail'), main = $('.main'), cmds = $('#cmds'), page = $('.page');
  if (!rail || !main || !cmds) return;
  const clean = el => { const c = el.cloneNode(true); $$('.alt,.tag', c).forEach(x => x.remove()); return c.textContent.replace(/\s+/g, ' ').trim(); };
  const tagsOf = el => $$('.tag', el).map(t => t.textContent.trim());
  const views = [], units = [];
  const sections = $$('.sec', main);
  const secOf = id => $('#' + id);

  // 1) หัวข้อก่อนคำสั่ง
  const add = v => { v.text = (v.label + ' ' + (v.extra || '') + ' ' + v.els.map(e => e.textContent).join(' ')).toLowerCase().replace(/\s+/g, ' '); views.push(v); return v; };
  const secView = (id, bucket) => { const s = secOf(id); if (s) add({ id, label: clean($('h2', s)), els: [s], sec: s, bucket, sectionRoot: true }); };
  secView('changelog', 'อัปเดต');
  ['how', 'eyes', 'pid'].forEach(id => secView(id, 'เริ่มต้น'));

  // 2) คำสั่ง: แบ่งตาม group-h
  const h2 = $('h2', cmds); units.push(h2);
  let curG = add({ id: 'cmds', label: 'ภาพรวมคำสั่ง', els: [h2], sec: cmds, bucket: 'คำสั่ง', kind: 'grp', cmdsRoot: true });
  const walk = el => {
    for (const c of [...el.children]) {
      if (c === h2) continue;
      if (c.tagName === 'DIV' && c.classList.contains('lvl-hi') && $('.group-h', c)) { walk(c); continue; }
      units.push(c);
      const hi = !!c.closest('.lvl-hi');
      if (c.classList.contains('group-h')) {
        curG = add({ id: c.id, label: clean(c), tags: tagsOf(c), els: [c], sec: cmds, bucket: 'คำสั่ง', kind: 'grp', hi });
      } else if (c.classList.contains('cmd')) {
        const h = $('h3', c), alt = $('.alt', h);
        const ends = $('.ends', c);
        add({ id: c.id, label: clean(h), extra: alt ? alt.textContent : '', tags: tagsOf(h), els: [c], sec: cmds, bucket: 'คำสั่ง', kind: 'cmd', hi,
          end: ends && ends.classList.contains('time') ? 'time' : ends && ends.classList.contains('sensor') ? 'sensor' : '', endText: ends ? ends.textContent.replace(/\s+/g, ' ').trim() : '', group: curG });
      } else { curG.els.push(c); }
    }
  };
  walk(cmds);
  // หน้ากลุ่ม: ตารางรายการคำสั่งในกลุ่ม
  const idxRow = w => {
    const use = $('.use dd', w.els[0]); const a = document.createElement('a'); a.className = 'gi'; a.href = '#' + w.id; if (w.hi) a.classList.add('lvl-hi');
    const b = document.createElement('b'); b.textContent = w.label; const sp = document.createElement('span'); sp.textContent = use ? use.textContent.trim() : '';
    const em = document.createElement('em'); em.textContent = w.endText.replace(/^หยุดเมื่อ\s*|^ไม่แนะนำ\s*/, w.endText.startsWith('ไม่แนะนำ') ? 'ไม่แนะนำ: ' : ''); if (w.end) em.className = w.end;
    a.append(b, sp, em); return a;
  };
  views.filter(v => v.kind === 'grp').forEach(g => {
    const kids = views.filter(w => w.kind === 'cmd' && w.group === g);
    const box = document.createElement('div'); box.className = 'gidx';
    if (g.cmdsRoot) { views.filter(w => w.kind === 'grp' && !w.cmdsRoot).forEach(w => { const a = document.createElement('a'); a.className = 'gi' + (w.hi ? ' lvl-hi' : ''); a.href = '#' + w.id; const b = document.createElement('b'); b.textContent = w.label.replace(/\s*\(.*$/, ''); const sp = document.createElement('span'); sp.textContent = views.filter(k => k.group === w).map(k => k.label).join('  '); sp.className = 'mono'; a.append(b, sp); box.appendChild(a); }); }
    else kids.forEach(w => box.appendChild(idxRow(w)));
    if (box.children.length) { g.els.push(box); g.els[0].after ? (g.els[0].parentNode.insertBefore(box, g.els[1] && g.els[1] !== box ? g.els[1] : null)) : 0; units.push(box); }
  });
  views.forEach(v => { if (v.kind === 'grp') v.text = (v.label + ' ' + v.els.map(e => e.textContent).join(' ')).toLowerCase().replace(/\s+/g, ' '); });

  // 3) หัวข้อหลังคำสั่ง
  ['play', 'tips', 'cheat'].forEach(id => secView(id, 'ฝึกและสรุป'));
  views.forEach(v => { if (!v.sec) v.sec = v.els[0]; });

  // ---------- เมนูซ้าย ----------
  const wrap = document.createElement('div'); wrap.className = 'bn-wrap open';
  const setOpen = b => { wrap.classList.toggle('open', b); tgl.setAttribute('aria-expanded', String(b)); requestAnimationFrame(() => cur && moveInd(cur.a)); };
  wrap.innerHTML = '<button type="button" class="bn-toggle" aria-expanded="true">เมนูคู่มือ</button><div class="bn-search"><input id="bnq" type="search" placeholder="ค้นหาคำสั่ง เช่น FL, เลี้ยว, ดัน" aria-label="ค้นหาคำสั่ง" autocomplete="off"><kbd aria-hidden="true">/</kbd></div><nav class="bn" id="bnav" aria-label="สารบัญคู่มือ"></nav>';
  rail.appendChild(wrap);
  const brand = document.createElement('div'); brand.className = 'bn-brand'; brand.setAttribute('aria-hidden', 'true'); brand.textContent = 'คู่มือหุ่น TRT';
  rail.insertBefore(brand, rail.firstChild);
  const tgl = $('.bn-toggle', wrap); tgl.addEventListener('click', () => setOpen(!wrap.classList.contains('open')));
  const bn = $('#bnav', wrap), q = $('#bnq', wrap);
  let lastB = '';
  views.forEach(v => {
    if (v.bucket !== lastB) { const h = document.createElement('div'); h.className = 'bn-h'; h.textContent = v.bucket; h.dataset.bucket = v.bucket; bn.appendChild(h); lastB = v.bucket; }
    const a = document.createElement('a'); a.href = '#' + v.id; a.className = v.kind === 'cmd' ? 'cmdi' : 'grp';
    if (v.hi) a.classList.add('lvl-hi');
    a.textContent = v.kind === 'grp' && !v.cmdsRoot ? v.label.replace(/\s*\(.*$/, '') : v.label;
    if (v.kind === 'cmd') { a.textContent = v.label; if (v.end === 'time') { const t = document.createElement('span'); t.className = 't sl'; t.textContent = 'เวลา'; t.title = 'หยุดเมื่อหมดเวลา'; a.appendChild(t); } }
    if (v.tags && v.tags.includes('ใหม่')) { const t = document.createElement('span'); t.className = 't'; t.textContent = 'ใหม่'; a.appendChild(t); }
    if (v.kind === 'cmd' && v.group && v.group.id === 'g-back') a.title = 'ไม่แนะนำ';
    v.a = a; bn.appendChild(a);
    if (v.kind === 'grp' && !v.cmdsRoot) a.classList.add('gh');
  });
  const ind = document.createElement('i'); ind.className = 'bn-ind nt'; bn.prepend(ind);
  const reduced = matchMedia('(prefers-reduced-motion: reduce)');
  if (!reduced.matches) {
    bn.classList.add('boot');
    [...bn.children].filter(c => c !== ind).slice(0, 40).forEach((c, i) => c.style.setProperty('--n', i));
    setTimeout(() => bn.classList.remove('boot'), 1500);
  }
  const none = document.createElement('div'); none.className = 'bn-none'; none.hidden = true; none.textContent = 'ไม่เจอคำสั่งที่ตรงกัน'; bn.appendChild(none);

  // ---------- ปุ่มก่อนหน้า / ถัดไป ----------
  const pager = document.createElement('div'); pager.className = 'pager';
  pager.innerHTML = '<button type="button" class="pv"><small>‹ ก่อนหน้า</small><span></span></button><button type="button" class="nx"><small>ถัดไป ›</small><span></span></button>';
  main.appendChild(pager);

  // ---------- แสดงผล ----------
  let cur = null;
  const hiOn = () => root.classList.contains('lvl-hi-on');
  const visible = () => views.filter(v => !v.a.hidden && (!v.hi || hiOn()));
  let indTop = null, indT;
  function moveInd(a) {
    if (!a || !a.offsetParent) return;
    const top = a.offsetTop, h = a.offsetHeight;
    const place = (t, hh) => { ind.style.height = hh + 'px'; ind.style.transform = 'translateY(' + t + 'px)'; };
    clearTimeout(indT);
    if (indTop === null || reduced.matches || ind.classList.contains('nt') || Math.abs(top - indTop) < 2) place(top, h);
    else { const lo = Math.min(top, indTop); place(lo, Math.abs(top - indTop) + h); indT = setTimeout(() => place(top, h), 170); }
    indTop = top; ind.classList.add('on');
    if (ind.classList.contains('nt')) requestAnimationFrame(() => requestAnimationFrame(() => ind.classList.remove('nt')));
  }
  let vinT;
  function animateIn(v) {
    if (reduced.matches) return;
    $$('.vin').forEach(e => { e.classList.remove('vin'); e.style.removeProperty('--i'); });
    void main.offsetWidth;
    const t = [];
    v.els.forEach(e => { if (e.matches('.cmd,.sec,.gidx')) t.push(...e.children); else t.push(e); });
    t.push(pager);
    t.forEach((e, i) => { e.style.setProperty('--i', Math.min(i, 9)); e.classList.add('vin'); });
    main.classList.remove('scan'); void main.offsetWidth; main.classList.add('scan');
    clearTimeout(vinT);
    vinT = setTimeout(() => t.forEach(e => { e.classList.remove('vin'); e.style.removeProperty('--i'); }), 1600);
  }
  const nowEl = $('#heroNow');
  if (nowEl && !reduced.matches) new MutationObserver(() => { nowEl.classList.remove('flash'); void nowEl.offsetWidth; nowEl.classList.add('flash'); }).observe(nowEl, { childList: true, characterData: true, subtree: true });
  function show(v, target, quiet) {
    cur = v;
    units.forEach(e => e.classList.add('bv-off'));
    sections.forEach(s => s.classList.add('bv-off'));
    v.sec.classList.remove('bv-off');
    v.els.forEach(e => e.classList.remove('bv-off'));
    views.forEach(x => x.a.removeAttribute('aria-current'));
    v.a.setAttribute('aria-current', 'page');
    moveInd(v.a);
    const list = visible(), i = list.indexOf(v);
    const set = (btn, w) => { btn.disabled = !w; if (w) { $('span', btn).textContent = w.label; btn.dataset.go = w.id; btn.classList.toggle('mono', w.kind === 'cmd'); $('span', btn).className = w.kind === 'cmd' ? '' : 'lbl-t'; } };
    set($('.pv', pager), list[i - 1]); set($('.nx', pager), list[i + 1]);
    document.title = v.label + ' · คู่มือหุ่น TRT';
    if (matchMedia('(max-width:900px)').matches) setOpen(false);
    if (quiet) { window.dispatchEvent(new Event('resize')); return; }
    animateIn(v);
    requestAnimationFrame(() => {
      const top = page.getBoundingClientRect().top + scrollY;
      if (target && target !== v.els[0]) target.scrollIntoView({ block: 'start' });
      else if (scrollY > top || matchMedia('(max-width:900px)').matches === false) scrollTo({ top: Math.max(top - 0, 0), behavior: 'auto' });
      v.a.scrollIntoView({ block: 'nearest' });
      window.dispatchEvent(new Event('resize'));
    });
  }
  function find(id) {
    let v = views.find(x => x.id === id); if (v) return [v, null];
    const el = id && document.getElementById(id); if (!el) return [null, null];
    v = views.find(x => x.els.some(e => e === el || e.contains(el))); return [v || null, el];
  }
  function route(first) {
    const id = decodeURIComponent(location.hash.slice(1));
    let [v, t] = find(id);
    if (!v) { if (!first) return; v = views[0]; t = null; }
    if (v.hi && !hiOn()) { root.classList.add('lvl-hi-on'); $$('.lvl-seg button').forEach(b => b.setAttribute('aria-pressed', String(b.dataset.lvl === 'hi'))); try { localStorage.setItem('trt-level', 'hi'); } catch (e) {} }
    if (first && !location.hash) { cur = v; showQuiet(v); return; }
    show(v, t);
  }
  function showQuiet(v) { show(v, null, true); }

  // ลิงก์ # ทั้งหน้าเปิดผ่านตัวนี้
  document.addEventListener('click', e => {
    const a = e.target.closest('a[href^="#"]'); if (!a || e.defaultPrevented || e.metaKey || e.ctrlKey) return;
    const id = decodeURIComponent(a.getAttribute('href').slice(1)); if (!id || !find(id)[0]) return;
    e.preventDefault(); if (location.hash !== '#' + id) history.pushState(null, '', '#' + id); route(false);
  });
  pager.addEventListener('click', e => { const b = e.target.closest('button'); if (!b || !b.dataset.go) return; history.pushState(null, '', '#' + b.dataset.go); route(false); });
  window.addEventListener('popstate', () => route(false));
  window.addEventListener('hashchange', () => route(false));

  // ค้นหา
  const filter = () => {
    const s = q.value.trim().toLowerCase(); let any = false;
    views.forEach(v => { const ok = !s || v.text.includes(s) || v.label.toLowerCase().includes(s); v.a.hidden = !ok; });
    $$('.bn-h', bn).forEach(h => { h.hidden = !views.some(v => v.bucket === h.dataset.bucket && !v.a.hidden && (!v.hi || hiOn())); });
    any = views.some(v => !v.a.hidden && (!v.hi || hiOn())); none.hidden = any;
    if (cur) requestAnimationFrame(() => moveInd(cur.a));
  };
  q.addEventListener('input', filter);
  q.addEventListener('keydown', e => {
    if (e.key === 'Enter') { const v = visible().find(x => x.kind === 'cmd') || visible()[0]; if (v) { history.pushState(null, '', '#' + v.id); route(false); } }
    if (e.key === 'Escape') { q.value = ''; filter(); q.blur(); }
  });
  document.addEventListener('keydown', e => { if (e.key === '/' && !/INPUT|TEXTAREA|SELECT/.test(document.activeElement.tagName)) { e.preventDefault(); setOpen(true); q.focus(); q.select(); } });

  // สลับระดับ: ถ้าหน้านี้เป็นของระดับสูงแล้วสลับเป็นกลาง กลับไปภาพรวม
  document.addEventListener('click', e => {
    if (!e.target.closest('.lvl-seg button')) return;
    setTimeout(() => { filter(); if (cur && cur.hi && !hiOn()) { history.pushState(null, '', '#cmds'); route(false); } else if (cur) show(cur); }, 30);
  });
  const mq = matchMedia('(max-width:900px)'); const sync = () => { setOpen(true); }; mq.addEventListener('change', sync);

  // ---------- ลูกเล่นเพิ่ม ----------
  // หัวข้อแตกเป็นคำ (ตัดคำไทยด้วย Intl.Segmenter) ให้ลอยขึ้นทีละคำ
  if (!reduced.matches && 'Segmenter' in Intl) {
    const seg = new Intl.Segmenter('th', { granularity: 'word' });
    const split = el => {
      let n = 0;
      [...el.childNodes].forEach(nd => {
        if (nd.nodeType !== 3 || !nd.textContent.trim()) return;
        const frag = document.createDocumentFragment();
        for (const { segment } of seg.segment(nd.textContent)) {
          if (!segment.trim()) { frag.append(segment); continue; }
          const w = document.createElement('span'); w.className = 'w'; w.style.setProperty('--w', Math.min(n++, 12)); w.textContent = segment; frag.append(w);
        }
        nd.replaceWith(frag);
      });
    };
    $$('.sec > h2, .group-h, .cmd > header > h3', main).forEach(split);
  }

  // แสงไฟฉายตามเมาส์บนแผง
  const SPOT = '.sim, .explore, .play, .gi, .pager button, .lvl-bar, .btn';
  if (matchMedia('(pointer: fine)').matches && !reduced.matches) {
    let raf = 0, last;
    document.addEventListener('pointermove', e => {
      last = e; if (raf) return;
      raf = requestAnimationFrame(() => {
        raf = 0; const el = last.target.closest && last.target.closest(SPOT); if (!el) return;
        const r = el.getBoundingClientRect(); el.style.setProperty('--mx', (last.clientX - r.left) + 'px'); el.style.setProperty('--my', (last.clientY - r.top) + 'px');
      });
    }, { passive: true });
  }

  // คลื่นตอนกดปุ่ม
  const RIP = '.btn, .seg button, .pager button, .explore .sens button, .explore .presets button';
  document.addEventListener('pointerdown', e => {
    if (reduced.matches) return;
    const b = e.target.closest(RIP); if (!b || b.disabled) return;
    const r = b.getBoundingClientRect(), d = document.createElement('span');
    d.className = 'rip'; d.style.left = (e.clientX - r.left) + 'px'; d.style.top = (e.clientY - r.top) + 'px';
    d.style.setProperty('--s', Math.ceil(Math.hypot(r.width, r.height) / 4));
    b.appendChild(d); setTimeout(() => d.remove(), 650);
  });

  // สลับเป็นระดับสูง: รายการใหม่ไหลเข้าทีละอัน
  document.addEventListener('click', e => {
    if (!e.target.closest('.lvl-seg button') || reduced.matches) return;
    setTimeout(() => {
      if (!hiOn()) return;
      const items = $$('#bnav a.lvl-hi').filter(a => a.offsetParent);
      items.forEach((a, i) => { a.style.setProperty('--n', i); a.classList.add('lvlin'); });
      setTimeout(() => items.forEach(a => a.classList.remove('lvlin')), 1200);
    }, 40);
  });

  filter(); route(true);
})();
