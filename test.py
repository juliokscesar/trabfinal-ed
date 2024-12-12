import numpy as np

vals = [0,1]
probs = [0.43, 0.57]
gen = np.random.choice(vals, size=100, p=probs)
print(", ".join([str(x) for x in gen[:gen.size//2]]))
print(", ".join([str(x) for x in gen[gen.size//2:]]))

