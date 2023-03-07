from turtle import color
import matplotlib.pyplot as plt
import numpy as np
from math import log

# run by calling "python plots.py"

def bar_chart(categories, values, title, x_label, y_label):

    plt.style.use('ggplot')
    cat_pos = np.arange(len(categories))
    plt.xticks(cat_pos, categories)
    plt.bar(cat_pos, values)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)
    plt.show()


def bar_chart_comp(categories, values_local, values_shared, title, x_label, y_label):

    x = np.arange(len(categories))
    width = 0.4

    fig, ax = plt.subplots()
    rects1 = ax.bar(x - width/2, values_local, width, label='Encryption')
    rects2 = ax.bar(x + width/2, values_shared, width, label='No Encryption')

    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_title(title)
    ax.set_xticks(x, categories)
    ax.legend()

    ax.bar_label(rects1, padding=3)
    ax.bar_label(rects2, padding=3)

    fig.tight_layout()

    plt.show()

if __name__ == '__main__':

    load_categories = ['A','B','C','D','E']
    run_categories = ['A \n (50% read, 50% update)','B \n (95% read, 5% update)','C\n (100% read)','D\n (95% read, 5% insert)','E\n (95% scan, 5% insert)']

    ### Bar Chart for the performance evaluation of a shared table
    # values_shared = [0.5, 3.7, 447.2, 8.9, 2.8]
    # values_local = [106.1, 500.5, 544.4, 243.6, 215.8]

    load_values_unencrypt = [97.63, 100.20, 92.54, 98.52, 89.96]
    load_values_encrypt = [6.72, 6.69, 6.62, 6.46, 6.61]

    run_values_unencrypt = [140.37, 205.93, 218.91, 183.35, 3.94]
    run_values_encrypt = [3.39, 3.44, 3.13, 3.40, 1.76]

    bar_chart_comp(load_categories, load_values_encrypt, load_values_unencrypt, "Encryption vs. No Encryption - Insert Phase (STUB)", "\n YCSB Workload", " Throughput (ops/sec)")

    bar_chart_comp(run_categories, run_values_encrypt, run_values_unencrypt, "Encryption vs. No Encryption - Run Phase (STUB)", "\n YCSB Workload", " Throughput (ops/sec)")