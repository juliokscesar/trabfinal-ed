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


def generate_binary_time_series(
        length=1000,
        p_initial=0.5,
        noise_prob=0.05,
        trend_prob=0.01,
        periodic_pattern=None,
        burst_prob=0.1,
        markov_order=1
):
    """
    Generate a random realistic binary time series.

    Parameters:
    - length (int): Length of the time series.
    - p_initial (float): Initial probability of starting with 1.
    - noise_prob (float): Probability of random bit flips.
    - trend_prob (float): Probability of maintaining the previous state (for trends).
    - periodic_pattern (list[int]): A repeating binary pattern to inject periodically.
    - burst_prob (float): Probability of a burst (sustained flips) occurring.
    - markov_order (int): Order of the Markov chain to model dependencies.

    Returns:
    - np.ndarray: Generated binary time series.
    """
    # Initialize the series
    series = np.zeros(length, dtype=int)
    series[0] = np.random.rand() < p_initial

    # Generate series based on trends and Markov dependencies
    for i in range(1, length):
        # Apply trend/memory with trend_prob
        if np.random.rand() < trend_prob:
            series[i] = series[i - 1]  # Maintain previous state
        else:
            series[i] = np.random.randint(2)  # Random state

        # Add noise (bit flip)
        if np.random.rand() < noise_prob:
            series[i] = 1 - series[i]

        # Add bursts (sustained flipping)
        if np.random.rand() < burst_prob:
            burst_length = np.random.randint(2, 6)  # Random burst length
            series[i:i + burst_length] = 1 - series[i - 1]

    # Inject periodic pattern if provided
    if periodic_pattern is not None:
        pattern_length = len(periodic_pattern)
        for start in range(0, length, pattern_length * 10):  # Inject pattern every 10 periods
            series[start:start + pattern_length] = periodic_pattern

    return series

def main():
    #pat = random_series([0,1], [0.3, 0.7], 10)
    pat = [0,0,1]
    gen = osc_series(1500, pat)
    #gen = random_series([0,1], [0.3, 0.7], 2000)
    # gen = generate_binary_time_series(
    #     length=2000,
    #     p_initial=0.64,
    #     noise_prob=0.04,
    #     trend_prob=0.01,
    #     periodic_pattern=None,
    #     burst_prob=0.1,
    #     markov_order=1,
    # )
    save_series("test.dat", gen)

if __name__ == "__main__":
    main()
