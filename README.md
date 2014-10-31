Public-Key Cryptography with Ring-LWE Encryption
========
This project performs public-key cryptography that is post-quantum secure. Public-key crypto uses two keys, one public and one private, that are mathematically linked. The public key can be used to encrypt a plaintext message, after which the encrypted message can be decrypted by the private key.

The underlying security is based on the Ring-LWE problem. It is capable of providing either 128-bit security or 256-bit security. Recent advances in post-quantum cryptography has highlighted the efficiency of using these schemes. This project aims to provide a working example of the underlying principles of Ring-LWE cryptography, and also to demonstrate how to make efficient implementations for this scheme.

Post-Quantum Cryptography
=========
Present day public-key schemes rely on the difficulty of performing factoring or the discrete logarithm for providing security, authenticity and privacy. For sufficiently large key sizes, these public-key cyptosystems are practically unsolvable using present day computers, super computers, or special hardware clusters. Peter Shor has proposed an algorithm for integer factorization, and this algorithm has been modified to be able to solve the elliptic curve discrete logarithm problem (ECDLP). Shor’s algorithms only runs on powerful quantum computers to solve the factoring or the discrete logarithm problems in polynomial time. 

Present day quantum computers are not yet capable of running Shor's algorithm, but we nevertheless need to think about the future. It is well-known that large government organizations (e.g., the United States' NSA and British Intelligence Agency MI5) are collecting vast amounts of communications data. This data is stored in large storage facilities where it might one day be analyzed, and all the public-key algorithms which was used, will become broken, allowing large amounts of sensitive information to be decrypted and analyzed.

Supported Parameters Sets
==========
This project supports two security parameter sets:

<table>
    <tr>
        <th>Security</th>
	<th>Number of coefficients (n)</th>
	<th>Modulus (q)</th>
	<th>Sigma</th>
    </tr>
    <tr>
	<td>128</td>
	<td>256</td>
	<td>7681</td>
	<td>11.31/sqrrt(2*pi)</td>
    </tr>
    <tr>
	<td>256</td>
	<td>512</td>
	<td>12289</td>
	<td>12.18/sqrrt(2*pi)</td>
    </tr>
</table>

Further Reading
==========
For a simple explanation of this work see "Efficient Software Implementation of Ring-LWE", available at https://eprint.iacr.org/2014/725.pdf. 

For a more detailed mathematical explanation of the Ring-LWE encryption scheme see "Compact Ring-LWE Cryptoprocessor", available at http://www.cosic.esat.kuleuven.be/publications/article-2444.pdf.

TODO
==========
<ul>
<li>Finish the file handling functionality</li>
<li>Add command-line parameter support</li>
<li>Convert lwe.c and lwe.h to use uint16_t instead of uint32_t</li>
</ul>

