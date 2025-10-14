

app
  λa λb app
    74{c, d} = a in c
    app d b
  λe λf app
    75{g, h} = e in g
    app h f

// applam:

λa app
  0{b, c} =
    λd λe app
      1{f, g} = d in f
      app g e
    in
    b
  app c a






// duplam:

λa app
  λh 0{b, c} =
    λe app
      1{f, g} = sup h i in f
      app g e
    in b
  app λi c a



// applam

λa 0{b, c} =
    λd app
      1{e, f} =
        sup
          (app (λi c a))
          i
        in e
      app f d
    in b
  

// duplam

λa λh 0{b d} =
  app
    1{e, f} =
      sup0
        (app (λg λc d a))
        g
      in e
    app f sup h c
  in b

// dupsup




λa λh 0{b d} =
  app
    sup0
      1{i k} =
        (app (λg λc d a)) in i
      1{j l} = g in j
    app
      sup0 k l
      sup0 h c
  in b


// app sup


λa λ b 0{c d} =
  sup 0
    app
      1{e f} =
        (app (λg λh d a)) in e
      0{i j} =
        app
          sup0
            f
            1{k l} = g in l
          sup0 b h
      in i
    app k j
  in c



// dup sup


λa λb app
  120{c, d} =
    app
      λe λf app
        120{g, h} = e in g
        119{i, j} =
          app
            sup119 d h
            sup119 b f
          in j
      a
    in c
  i


// app lam


λa λb app
  120{c, d} =
    λf app
      120{g, h} = a in g
      119{i, j} =
        app
          sup119 d h
          sup119 b f
        in j
    in c
  i

// dup lam


λa λb app
  // 120{c, d} =
  //   λf app
  //     120{g, h} = a in g
  //     119{i, j} =
  //       app
  //         sup119 d h
  //         sup119 b f
  //       in j
  //   in c

  

  i

// EXMAPLE
// DUP - LAM


app
  122{a, b} =
    λc c in a
  b


app
  λa {e g} = sup a b in e
  λb g



