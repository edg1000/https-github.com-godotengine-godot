#!/usr/bin/env python

import argparse     # for ArgumentParser
import subprocess   # for Popen
import tempfile     # for NamedTemporaryFile
import os           # for remove

class gnuplot(object):

    output = "result.png"

    script = """
             set terminal png size 1024, 768
             set output "{}.png"
             set title "re2 benchlog"
             set datafile separator ";"
             set grid x y
             set ylabel "MB/s"
             set autoscale
             plot """

    template = """'{}' using 1:5:xticlabels(2) with linespoints linewidth 3 title "{}",\\\n"""

    benchdata = dict()
    tempfiles = []

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        """
        remove all temporary files
        """

        for filename in self.tempfiles:
            os.remove(filename)

    def parse_re2_benchlog(self, filename):
        """
        parse the input benchlog and return a dictionary contain bench data
        """

        benchdata = self.benchdata

        with open(filename) as f:

            for raw in f.readlines():

                data = raw.split('\t')

                if len(data) == 4:

                    data = data[0].split('/') + data[1:]
                    data = list(map(str.strip, data))

                    if not benchdata.get(data[0]):
                        benchdata[data[0]] = [ data[1:] ]
                    else:
                        benchdata[data[0]].append(data[1:])

    def gen_csv(self):
        """
        generate temporary csv files
        """

        for name, data in self.benchdata.items():

            with tempfile.NamedTemporaryFile(delete=False) as f:

                for index, line in enumerate(data):
                    f.write('{};{}\n'.format(index, ';'.join(line)).encode())

                self.tempfiles.append(f.name)
                self.script = self.script + self.template.format(f.name, name)

    def run(self):
        self.gen_csv()
        script = self.script[:-3].format(self.output)
        command = subprocess.Popen(['gnuplot'], stdin=subprocess.PIPE)
        command.communicate(script.encode())


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='generate plots for benchlog')
    parser.add_argument('benchlog', type=str, help='benchlog generated by re2')
    args = parser.parse_args()

    try:
        subprocess.Popen(['gnuplot'], stdin=subprocess.PIPE)
    except FileNotFoundError:
        print('you can install "gnuplot" to generate plots automatically')
        exit(1)

    with gnuplot() as plot:
        plot.output = args.benchlog
        plot.parse_re2_benchlog(args.benchlog)
        plot.run()
