(File loop.l)
(define-loop-sequence-path macro quote list caddddr cadddddr cadddr caddr cadr)
(si:loop-sequence-elements-path lambda or si:loop-sequencer cons list* cadr cdr car setq error loop-simple-error cadar cddr rplacd null not cond list quote si:loop-named-variable let)
(si:loop-sequencer lambda subst rplaca rplacd caddr member loop-typed-arith cons setf push cdr not gensym loop-constantp if loop-maybe-bind-form eq si:loop-tequal append loop-make-variable quote car list error loop-simple-error memq si:loop-tmember cond cadar caar setq null do loop-make-iteration-variable and loop-typed-init or add1 let)
(define-loop-path macro append nreverse cdr quote car cons null liszt-internal-do mapcar let list atom cond if setq cddr cadr)
(loop-add-path lambda cdr car return caar eq cond null do assq si:loop-tassoc delq cons setq)
(loop-gather-preps lambda nreverse return caar assq si:loop-tassoc cadr symbolp cddr null not and atom quote error loop-simple-error or eq si:loop-tequal loop-get-form cdr prog1 pop loop-pop-source list cons setq setf push memq si:loop-tmember cond car do)
(loop-for-being lambda cddr reverse nconc loop-make-iteration-variable atom Internal-bcdcall getdisc bcdp cxr getd symbolp cdddr caddr loop-gather-preps nreconc funcall memq si:loop-tmember list error loop-simple-error loop-get-form prog1 pop loop-pop-source cadr not cdr return caar null do assq si:loop-tassoc and cons setf push car quote setq eq si:loop-tequal or cond if let)
(si:loop-named-variable lambda delq setq gensym cdr car return caar eq cond null do assq si:loop-tassoc let)
(loop-for-arithmetic lambda loop-gather-preps cons list quote or si:loop-sequencer)
(loop-list-stepper lambda cons last rplacd loop-make-variable loop-make-iteration-variable and cadr gensym list memq not atom or if quote loop-get-form cdr setq setf prog1 pop loop-pop-source car eq si:loop-tequal cond let)
(loop-for-first lambda loop-get-form list quote cons loop-make-iteration-variable cdr setq setf prog1 pop loop-pop-source error loop-simple-error car eq si:loop-tequal or)
(loop-for-equals lambda loop-make-setq loop-emit-body let loop-get-form list quote cons loop-make-iteration-variable cdr setq setf prog1 pop loop-pop-source car eq si:loop-tequal cond)
(loop-when-it-variable lambda gensym loop-make-variable setq or)
(loop-do-repeat lambda loop-typed-arith list quote loop-get-form gensym loop-make-variable let)
(loop-do-for lambda cddr cadr lexpr-funcall quote list error loop-simple-error return caar eq cond null do assq si:loop-tassoc or loop-get-form loop-optional-type cdr setq setf car prog1 pop loop-pop-source let)
(loop-hack-iteration lambda prog1 pop loop-pop-source loop-bind-block delq quote loop-make-conditionalization nreverse liszt-internal-do mapcar nreconc loop-make-setq loop-make-psetq loop-end-testify return caar null assq si:loop-tassoc eq si:loop-tequal not error loop-simple-error cond if caddr list let or cdr append loop-copylist* nconc cons setf push car and cddr cadr apply setq do)
(si:loop-simplep-1 lambda *throw macroexpand not memq Internal-bcdcall getdisc bcdp cxr getd and funcall fixp if quote get symbolp si:loop-simplep-1 + setq null cdr do car eq atom loop-constantp cond let)
(si:loop-simplep lambda <& < and si:loop-simplep-1 let quote *catch null cond if)
(loop-do-thereis lambda loop-get-form loop-when-it-variable quote list loop-emit-body)
(loop-do-always lambda setq setf push quote cons loop-emit-body loop-get-form let)
(loop-do-with lambda loop-bind-block if loop-make-variable error loop-simple-error not and return caar null assq si:loop-tassoc or loop-get-form eq si:loop-tequal cond cdr setf car prog1 pop loop-pop-source setq do)
(loop-do-when lambda nconc and cddr car cons list* loop-when-it-variable quote list setq cadr eq si:loop-tequal cond loop-get-form let)
(loop-do-while lambda quote cons loop-pseudo-body list error loop-simple-error and loop-get-form)
(loop-cdrify lambda - setq quote eq =& = list bigp not cdr null and zerop cond if <& < length do)
(loop-do-collect lambda cddr loop-cdrify nreverse si:loop-tmember member if loop-typed-arith caseq loop-emit-body list* loop-make-iteration-variable cons push or gensym loop-make-variable rplaca atom ferror cadr and list not return caar null do assq cdr setf prog1 pop loop-pop-source car eq si:loop-tequal loop-optional-type loop-get-form setq error loop-simple-error quote memq cond let)
(loop-do-return lambda loop-get-form quote list loop-pseudo-body)
(loop-do-do lambda loop-get-form loop-emit-body)
(loop-do-finally lambda loop-get-form cons setq setf push)
(loop-nodeclare lambda append error loop-simple-error typep eq and or cdr setq setf car prog1 pop loop-pop-source)
(loop-do-initially lambda loop-get-form cons setq setf push)
(loop-emit-body lambda loop-pseudo-body quote setq)
(loop-pseudo-body lambda cons setq setf push null not cond loop-make-conditionalization)
(loop-make-conditionalization lambda do nreverse cadr quote list error loop-simple-error cddr let cdr setq setf prog1 pop loop-pop-source eq si:loop-tequal cons ncons car last rplacd null not cond)
(loop-optional-type lambda cdr setq setf prog1 pop loop-pop-source memq si:loop-tmember quote get data-type? atom or null not and car let)
(loop-maybe-bind-form lambda gensym loop-make-variable loop-constantp cond if)
(loop-constantp lambda car eq atom not and stringp memq numberp or)
(loop-declare-variable lambda list error loop-simple-error cdr car loop-declare-variable typep eq object-that-cares-p variable-declarations append setq quote get data-type? memq symbolp null or cond)
(loop-make-iteration-variable lambda loop-make-variable quote let)
(loop-make-variable lambda loop-make-variable list* gensym let loop-typed-init loop-declare-variable symbolp or cdr car return caar eq do assq error loop-simple-error memq if atom list cons setq setf push not null cond)
(loop-typed-init lambda eq memq si:loop-tmember car setq initial-value quote get data-type? cond)
(loop-typed-arith lambda quote sublis eq si:loop-tequal cond if)
(loop-get-form lambda nreverse quote null cond if atom cdr setq setf car prog1 pop loop-pop-source cons ncons do)
(loop-bind-block lambda progn cons setq setf push null not cond)
(loop-translate-1 lambda list memq si:loop-tmember loop-hack-iteration cddr apply assq si:loop-tassoc loop-pop-source if cadr let return symbolp caar or atom not cond loop-make-desetq prog1 pop ncons nreverse nconc quote cons setf push loop-bind-block loop-optimize-duplicated-code-etc caadar error loop-simple-error null do cdr setq car eq and)
(loop-output-group lambda list not cond cons setf push quote gensym loop-make-variable setq let or and)
(loop-optimize-duplicated-code-etc lambda loop-output-group eq and return si:loop-simplep null not cond cdr car cons setf push quote delq nconc setq equal nreverse do)
(loop-end-testify lambda car nreverse setq cdr quote cons null cond if)
(loop-translate lambda loop-translate-1 displace)
(loop-finish macro quote)
(define-loop-macro macro quote subst error loop-simple-error cdr car return caar cond null do assq si:loop-tassoc eq or cadr)
(si:loop-tassoc macro quote list caddr cadr)
(si:loop-tmember macro quote list caddr cadr)
(si:loop-tequal macro quote list caddr cadr)
(loop-make-setq lambda loop-make-desetq and)
(loop-desetq-internal lambda cdr quote car loop-desetq-internal nconc list atom null cond)
(loop-make-desetq lambda loop-desetq-internal nconc gensym list* memq atom cadr setq cddr return symbolp car not and or quote null do cons cond if)
(loop-make-psetq lambda loop-make-psetq quote cadr cddr null cond if car list loop-make-setq and)
