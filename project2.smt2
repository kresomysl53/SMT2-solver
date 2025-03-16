(set-logic NIA)

(set-option :produce-models true)
(set-option :incremental true)

; Deklarace promennych pro vstupy
; ===============================

; Parametry
(declare-fun A () Int)
(declare-fun B () Int)
(declare-fun C () Int)
(declare-fun D () Int)
(declare-fun E () Int)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;; START OF SOLUTION ;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; XLOGIN: xholinp00

; Promenne
(declare-fun x () Int)
(declare-fun y () Int)
(declare-fun z () Int)
(declare-fun res () Int)

;D > 0 and E > 0
(assert (and (> D 0) (> E 0)))

;x = A * B - C * 5;
(assert (= x (- (* A B) (* C 5))))

; if (x + E < D + D) {
;    y = x + 3 * B;
;  } else {
;    y = x * C * 2;
;  }
;                   podminka                true              false
(assert (= y (ite   (< (+ x E) (+ D D))     (+ x (* 3 B))     (* (* x C) 2)   )))

; if (y - 5 <= C) {
;     z = x * A - y * B
;   } else if (x + 2 > D) {
;     z = (x + A) * (y + B)
;   } else {
;     z = x * B + y * A
;   }
;                   podminka1         true podminka1          false - else if         true else if          false else if 
(assert (= z (ite   (<= (- y 5) C)    (- (* x A) (* y B))     (ite  (> (+ x 2) D)     (* (+ x A)(+ y B))    (+ (* x B) (* y A))))))

; if (z <= D + E) {
;     return false
;   } else {
;     return true
;   }
;
;z > (D + E)
(assert (> z (+ D E)))

;nejmeni mozny vysledek
;pro vsechna D* a E*
(assert (forall ((D* Int) (E* Int))
    ;
    ;pokud    D* > 0 a E* > 0 a...
    (=>       (and(> D* 0) (> E* 0)
    ;
    ;x*
    (let ((x* (- (* A B) (* C 5))))
    ;
    ;y*             podminka                    true                false
    (let ((y* (ite  (< (+ x* E*) (+ D* D*))     (+ x* (* 3 B))      (* (* x* C) 2))))
    ;
    ;z*             podminka1        true podminka1          false podminka1 - else if   true else if            false else if 
    (let ((z* (ite  (<= (- y* 5) C)  (- (* x* A) (* y* B))   (ite (> (+ x* 2) D*)        (* (+ x* A) (+ y* B))   (+ (* x* B) (* y* A))))))
    ;
    ;z* > (D* + E*)
    (> z* (+ D* E*))))))
    ;
    ;tak D + E <= D* + E*
    (<= (+ D E) (+ D* E*)))))

; bonus:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;; END OF SOLUTION ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Testovaci vstupy
; ================

(echo "Test 1 - vstup A=1, B=1, C=3")
(echo "a) Ocekavany vystup je sat a D+E se rovna 17")
(check-sat-assuming (
  (= A 1) (= B 1) (= C 3)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 17, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 1) (= B 1) (= C 3) (distinct (+ D E) 17)
))

(echo "")
(echo "")

(echo "Test 2 - vstup A=5, B=2, C=5")
(echo "a) Ocekavany vystup je sat a D+E se rovna 18")
(check-sat-assuming (
  (= A 5) (= B 2) (= C 5)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 18, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 5) (= B 2) (= C 5) (distinct (+ D E) 18)
))

(echo "")
(echo "")

(echo "Test 3 - vstup A=100, B=15, C=1")
(echo "a) Ocekavany vystup je sat a D+E se rovna 2")
(check-sat-assuming (
  (= A 100) (= B 15) (= C 1)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 2, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 100) (= B 15) (= C 1) (distinct (+ D E) 2)
))

(echo "")
(echo "")

(echo "Test 4 - vstup A=5, B=5, C=3")
(echo "a) Ocekavany vystup je sat a D+E se rovna 2")
(check-sat-assuming (
  (= A 5) (= B 5) (= C 3)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 2, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 5) (= B 5) (= C 3) (distinct (+ D E) 2)
))

(echo "")
(echo "")

(echo "Test 5 - vstup A=1, B=1, C=1")
(echo "a) Ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 1) (= B 1) (= C 1)
))

(echo "")
(echo "")

(echo "Test 6 - vstup A=4, B=5, C=3")
(echo "a) Ocekavany vystup je sat a D+E se rovna 2")
(check-sat-assuming (
  (= A 4) (= B 5) (= C 3)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 2, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 4) (= B 5) (= C 3) (distinct (+ D E) 2)
))

(echo "")
(echo "")

(echo "Test 7 - vstup A=0, B=0, C=0")
(echo "a) Ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 0) (= B 0) (= C 0)
))

(echo "")
(echo "")

(echo "Test 8 - vstup A=100, B=-50, C=-75")
(echo "a) Ocekavany vystup je sat a D+E se rovna 4628")
(check-sat-assuming (
  (= A 100) (= B -50) (= C -75)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 4628, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 100) (= B -50) (= C -75) (distinct (+ D E) 4628)
))

(echo "")
(echo "")

(echo "Test 9 - vstup A=3, B=2, C=2")
(echo "a) Ocekavany vystup je sat a D+E se rovna 7")
(check-sat-assuming (
  (= A 3) (= B 2) (= C 2)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 7, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 3) (= B 2) (= C 2) (distinct (+ D E) 7)
))

(echo "")
(echo "")

(echo "Test 10 - vstup A=-100, B=-15, C=-90")
(echo "a) Ocekavany vystup je sat a D+E se rovna 977")
(check-sat-assuming (
  (= A -100) (= B -15) (= C -90)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 977, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A -100) (= B -15) (= C -90) (distinct (+ D E) 977)
))
