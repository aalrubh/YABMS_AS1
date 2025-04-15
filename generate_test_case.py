import numpy as np
import csv
import random

def choose_data_set(dataset):
    if dataset == "testing":
        n,m,p = 16,12,8
    elif dataset == "small":
        n,m,p = 121,180,115
    elif dataset == "medium":
        n,m,p = 550,620,480
    elif dataset == "large":
        n,m,p = 962,1012,1221
    elif dataset == "native":
        n,m,p = 2500,3000,2100
    else:
        raise Exception(f"Error: {dataset} is not appropriate, please choose one of the following: testing, small, medium, large, native.")

    print(f"Choice: {dataset}, the corresponding n,m,p are {n},{m},{p}")

    return n,m,p

def gen_matrices(dataset, n, m, p, seed, size):

    random.seed(seed)
    out_test = None
    out_golden = None

    try:
        out_test = open(f"{dataset}_test.csv", "w")
        out_golden = open(f"{dataset}_golden.csv", "w")

        testcase = csv.writer(out_test)
        golden = csv.writer(out_golden)

        for i in range(size):
            A = np.random.rand(n, m)
            B = np.random.rand(m, p)
            R = np.matmul(A, B)

            testcase.writerow(A.flatten())
            testcase.writerow(B.flatten())
            golden.writerow(R.flatten())

            if i != 0 and (i + 1) % 100 == 0:
                print(f"Done with {i+1} test cases")

    except FileNotFoundError as error:
        print(error)
    except Exception as error:
        print(error)
    finally:
        if out_test:
            out_test.close()
        if out_golden:
            out_golden.close()

def main():
    datasets = ['testing', 'small', 'medium', 'large', 'native']
    seed = 0xdeadbeef
    size = 1

    for dataset in datasets:
        n,m,p = choose_data_set(dataset)
        gen_matrices(dataset, n, m, p, seed, size)
        print(f"Generated {dataset} dataset with {size} tests cases with dimensions {n}, {m}, {p} successfully")
main()