/* Generated by Yosys 0.9+2406 (git sha1 3eb24809, clang 10.0.0-4ubuntu1 -fPIC -Os) */

(* top =  1  *)
(* src = "2bit_adder.v:2.1-14.10" *)
module adder2(\a[0] , \a[1] , \b[0] , \b[1] , ci, \sum[0] , \sum[1] , co);
  wire _00_;
  wire _01_;
  wire _02_;
  wire _03_;
  wire _04_;
  wire _05_;
  wire _06_;
  wire _07_;
  wire _08_;
  wire _09_;
  wire _10_;
  wire _11_;
  wire _12_;
  wire _13_;
  wire _14_;
  wire _15_;
  wire _16_;
  wire _17_;
  wire _18_;
  wire _19_;
  wire _20_;
  (* src = "2bit_adder.v:4.18-4.19" *)
  input \a[0] ;
  (* src = "2bit_adder.v:4.18-4.19" *)
  input \a[1] ;
  (* src = "2bit_adder.v:5.18-5.19" *)
  input \b[0] ;
  (* src = "2bit_adder.v:5.18-5.19" *)
  input \b[1] ;
  (* src = "2bit_adder.v:3.18-3.20" *)
  input ci;
  (* src = "2bit_adder.v:7.18-7.20" *)
  output co;
  (* src = "2bit_adder.v:6.18-6.21" *)
  output \sum[0] ;
  (* src = "2bit_adder.v:6.18-6.21" *)
  output \sum[1] ;
  INVX1 _21_ (
    .A(\a[0] ),
    .Y(_20_)
  );
  INVX1 _22_ (
    .A(\b[0] ),
    .Y(_00_)
  );
  INVX1 _23_ (
    .A(ci),
    .Y(_01_)
  );
  INVX1 _24_ (
    .A(\a[1] ),
    .Y(_02_)
  );
  INVX1 _25_ (
    .A(\b[1] ),
    .Y(_03_)
  );
  NAND2X1 _26_ (
    .A(\a[1] ),
    .B(\b[1] ),
    .Y(_04_)
  );
  NAND2X1 _27_ (
    .A(\a[0] ),
    .B(\b[0] ),
    .Y(_05_)
  );
  NAND2X1 _28_ (
    .A(_20_),
    .B(_00_),
    .Y(_06_)
  );
  NAND2X1 _29_ (
    .A(_05_),
    .B(_06_),
    .Y(_07_)
  );
  INVX1 _30_ (
    .A(_07_),
    .Y(_08_)
  );
  NAND2X1 _31_ (
    .A(ci),
    .B(_08_),
    .Y(_09_)
  );
  NAND2X1 _32_ (
    .A(_05_),
    .B(_09_),
    .Y(_10_)
  );
  INVX1 _33_ (
    .A(_10_),
    .Y(_11_)
  );
  NAND2X1 _34_ (
    .A(_02_),
    .B(_03_),
    .Y(_12_)
  );
  NAND2X1 _35_ (
    .A(_04_),
    .B(_12_),
    .Y(_13_)
  );
  INVX1 _36_ (
    .A(_13_),
    .Y(_14_)
  );
  NAND2X1 _37_ (
    .A(_10_),
    .B(_14_),
    .Y(_15_)
  );
  NAND2X1 _38_ (
    .A(_04_),
    .B(_15_),
    .Y(co)
  );
  NAND2X1 _39_ (
    .A(_01_),
    .B(_07_),
    .Y(_16_)
  );
  NAND2X1 _40_ (
    .A(_09_),
    .B(_16_),
    .Y(_17_)
  );
  INVX1 _41_ (
    .A(_17_),
    .Y(\sum[0] )
  );
  NAND2X1 _42_ (
    .A(_11_),
    .B(_13_),
    .Y(_18_)
  );
  NAND2X1 _43_ (
    .A(_15_),
    .B(_18_),
    .Y(_19_)
  );
  INVX1 _44_ (
    .A(_19_),
    .Y(\sum[1] )
  );
endmodule