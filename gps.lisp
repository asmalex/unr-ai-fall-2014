#|

This code is taken from chapter 4 of Peter Norvig's Paradigms of Artificial 
Intelligence Programming. The book is a classic. If you're reading this, you
should almost certainly go buy the book. Now. Even if you don't know or care
about lisp.

The program under consideration is the General Problem Solver, developed by
Alan Newell and Herbert Simon in 1957. Here's Simon's quote:

"It is not my aim to surprise or shock you ... But the simplest way I can
summarize is to say that there are now in the world machines that think, that
learn and create. Moreover, their ability to do these things is going to increase
rapidly until - in a visible future - the range of problems they can handle
will be coextensive with the range to which the human mind has been applied."

|#

;;; First, two helper functions
(defun compl (fn)
  "If FN returns y, then (complement FN) returns (not y)"
  #'(lambda (&rest args) (not (apply fn args))))

(defun find-all (item sequence &rest keyword-args
		 &key (test #'eql) test-not &allow-other-keys)
  "Find all those elements of sequence that match item,
   according to the keywords. Doesn't alter sequence."
  (if test-not
      (apply #'remove item sequence
	     :test-not (compl test-not) keyword-args)
      (apply #'remove item sequence
	     :test (compl test) keyword-args)))


;; Now, the GPS. The 4 functions below are a simplified version of the program,
;; but still convey the gist of what it was trying to achieve.

(defvar *state* nil "The current state: a list of conditions.")
(defvar *ops* nil "A list of available operators.")

(defstruct op "an operation"
	   (action nil)
	   (preconds nil)
	   (add-list nil)
	   (del-list nil))

(defun GPS (*state* goals *ops*)
  "General problem solver: achieve all goals using *ops*."
  (if (every #'achieve goals) 'solved))

(defun achieve (goal)
  "A goal is achieved if it already holds, or if there is
   an appropriate op for it that is applicable."
  (or (member goal *state*)
      (some #'apply-op
	    (find-all goal *ops* :test #'appropriate-p))))

(defun appropriate-p (goal op)
  "An op is appropriate to a goal if it is in its add list."
  (member goal (op-add-list op)))

(defun apply-op (op)
  "Print a message and update *state* if op is applicable."
  (when (every #'achieve (op-preconds op))
    (print (list 'executing (op-action op)))
    (setf *state* (set-difference *state* (op-del-list op)))
    (setf *state* (union *state* (op-add-list op)))
    t))

;; That's it!

;; Here is some data for the examples.
(defparameter *school-ops*
  (list
   (make-op :action 'drive-son-to-school
	    :preconds '(son-at-home car-works)
	    :add-list '(son-at-school)
	    :del-list '(son-at-home))
   (make-op :action 'shop-installs-battery
	    :preconds '(car-needs-battery shop-knows-problem shop-has-money)
	    :add-list '(car-works))
   (make-op :action 'tell-shop-problem
	    :preconds '(in-communication-with-shop)
	    :add-list '(shop-knows-problem))
   (make-op :action 'telephone-shop
	    :preconds '(know-phone-number)
	    :add-list '(in-communication-with-shop))
   (make-op :action 'look-up-number
	    :preconds '(have-phone-book)
	    :add-list '(know-phone-number))
   (make-op :action 'give-shop-money
	    :preconds '(have-money)
	    :add-list '(shop-has-money)
	    :del-list '(have-money))))


;; Here are three examples that show the system in action.
(defun ex1 ()
  (gps '(son-at-home car-needs-battery have-money have-phone-book)
       '(son-at-school)
       *school-ops*))

(defun ex2 ()
  (gps '(son-at-home car-needs-battery have-money)
       '(son-at-school)
       *school-ops*))

(defun ex3 ()
  (gps '(son-at-home car-works)
       '(son-at-school)
       *school-ops*))
