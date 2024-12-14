import numpy as np

def random_series(p0: float, p1: float, size: int) -> np.ndarray:
    if abs(1.0-(p0+p1)) > 1e-5:
        print("probabilities must add up to 1")
        return None
    vals = [0,1]
    probs = [p0, p1]
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
    gen = random_series(0.43, 1.0-0.43, 1500)
    save_series("test.dat", gen)

if __name__ == "__main__":
    main()
