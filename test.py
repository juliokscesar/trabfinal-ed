import numpy as np

def random_series(vals: list, probs: list, size: int) -> np.ndarray:
    if abs(1.0-sum(probs)) > 1e-5:
        print("probabilities must add up to 1")
        return None
    return np.random.choice(vals, size=size, p=probs).tolist()

def osc_series(length, pattern=None):
    if pattern is None:
        pattern = [0, 0, 1]

    repeated_pattern = (pattern * ((length // len(pattern)) + 1))[:length]
    return np.array(repeated_pattern)

def save_series(file: str, data):
    with open(file, "w") as f:
        for i, val in enumerate(data):
            f.write(str(val))
            if i != len(data)-1:
                f.write('\n')
def main():
    #pat = random_series([0,1], [0.3, 0.7], 10)
    #pat = [0,0,1,1]
    #gen = osc_series(1500, pat)
    gen = random_series([0,1], [0.43, 1-0.43], 2000)
    save_series("test.dat", gen)

if __name__ == "__main__":
    main()
