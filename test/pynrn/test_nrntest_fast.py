"""
Tests that used to live in the fast/ subdirectory of the
https://github.com/neuronsimulator/nrntest repository
"""
import math
from neuron import h
from neuron.tests.utils import cvode_enabled, cvode_use_long_double, num_threads

h.load_file("stdrun.hoc")


class Cell:
    def __init__(self, id):
        self.id = id
        self.soma = h.Section(name="soma", cell=self)
        self.soma.L = 10
        self.soma.diam = 100 / (math.pi * self.soma.L)
        self.soma.insert("hh")
        self.s = h.IClamp(self.soma(0.5))
        self.s.dur = 0.1 + 0.1 * id
        self.s.amp = 0.2 + 0.1 * id
        self.vv = h.Vector()
        self.vv.record(self.soma(0.5)._ref_v)
        self.tv = h.Vector()
        self.tv.record(h._ref_t)

    def pr(self, file=None):
        print("\n{}".format(self), file=file)
        assert self.tv.size() == self.vv.size()
        for t, v in zip(self.tv, self.vv):
            print("{:.20g} {:.20g}".format(t, v), file=file)

    def __str__(self):
        return "Cell[{:d}]".format(self.id)


def test_t13():
    """
    hh model, fixed step, and cvode with threads
    Note: full double precision identity between single and multiple
    threads is obtainable when cvode.use_long_double(1) is invoked
    (avoids different roundoff error due to different summation order
    in N_VWrmsNorm)
    """
    h.nrnunit_use_legacy(True)
    cells = [Cell(i) for i in range(2)]
    with num_threads(threads=1, parallel=True), open("temp13_py", "w") as ofile:
        print("fixed step", file=ofile)
        h.run()
        for cell in cells:
            cell.pr(ofile)
        with cvode_enabled(True), cvode_use_long_double(False):
            print("cvode", file=ofile)
            h.run()
            for cell in cells:
                cell.pr(ofile)


if __name__ == "__main__":
    test_t13()
