# Lagrangean-Relaxation-Party
A C++ program for solving Rectangular Partitions using Lagrangian Relaxation, which tries to be generic enough that you can easily build your own case.

This code is the result of my master's degree completed more than 20 years ago. Recently during the Pandemic I added Branch & Bound capabilities. It worked but I have to say I'm not a programmer anymore so there will be much better ways to do this.

I now share this code with the aim that students have a starting point and are able to implement a Lagrangian Relaxation for the problem they want, without going through all the challenges that this type of implementation offers.

I have started documenting how to reuse and extend the framework for other combinatorial optimization problems in [How to Extend the Framework](HOW_TO_EXTEND.md).

At the moment, the code is most naturally suited to binary problems with unit coefficients, especially formulations that look like set partitioning, set covering, or packing models. Adapting it to more general integer models may require extending the base classes.

I also have some problems with the code (a program is never truly finished) which I will share here. If you are an enthusiast, feel free to contribute. Also feel free to suggest new topics.

Here you have some links to my work. Unfortunately, my master's thesis is written in Portuguese (as you may have noticed, English is not one of my talents). The article is paid for, but I believe you can find a free version by searching Google.

-  [Master's thesis](https://repositorio.unicamp.br/Busca/Download?codigoArquivo=489487) - PDF File
-  [Article: Optimal Rectangular Partitions](https://onlinelibrary.wiley.com/doi/abs/10.1002/net.10058)
  
One more important information. Although this code was originally written in C++ using [Standard Template Library](https://en.wikipedia.org/wiki/Standard_Template_Library) in an old version of [emacs editor](https://www.gnu.org/software/emacs/) in the late 90s, it was adapted to run in Microsoft Visual Studio 2019:tm:.
