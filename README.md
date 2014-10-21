Ring-LWE Encryption
========
This project does post-quantum secure public-key cryptography. The underlying security is based on the Ring-LWE problem. It is capable of providing either 128-bit security or 256-bit security. Recent advances in post-quantum cryptography has highlighted the efficiency of using these schemes. This project aims to provide a working example of the underlying principles of Ring-LWE cryptography, and also to demonstrate how to make efficient implementations for this scheme.

Post-Quantum Cryptography
=========
Present day public-key schemes rely on the difficulty of performing factoring or the discrete logarithm for providing security, authenticity and privacy. For sufficiently large key sizes, these public-key cyptosystems are practically unsolvable
using present day computers or even using super computers or special hardware clusters. Peter Shor has proposed a quantum algorithm for integer factorization, and this algorithm has been modified to be able to solve the elliptic curve discrete logarithm problem (ECDLP). Shor’s algorithms only runs on powerful quantum computers to solve the factoring or the discrete logarithm problems in polynomial time.

Supported Parameters Sets
==========
This project supports two security parameter sets.

The 128-bit security parameter set: n=256, q=7681, sigma=11.31/sqrrt(2*pi)

The 256-bit security parameter set: n=512, q=12289, sigma=12.18/sqrrt(2*pi)

<table>
    <tr>
        <th>Security</th>
	<th>Number of coefficients (n)</th>
	<th>Modulus (q)</th>
	<th>Sigma</th>
    </tr>
</table>

Further Reading
==========
For a simple explanation of this work see "Efficient Software Implementation of Ring-LWE", available at https://eprint.iacr.org/2014/725.pdf. 

For a more detailed mathematical explanation of the Ring-LWE encryption scheme see "Compact Ring-LWE Cryptoprocessor", available at http://www.cosic.esat.kuleuven.be/publications/article-2444.pdf.
