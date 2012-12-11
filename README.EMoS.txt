EMoS - Elder Monitoring System

Este trabalho foi desenvolvido no âmbito da dissertação de mestrado em Engenharia Electrotécnica e de Computadores, 
entitulada "Monitorização de Pessoas em Ambiente Doméstico".

Resumo (PT)

O aumento constante da população idosa mundial tem criado uma enorme quantidade de
desafios ao desenvolvimento nacional, à sustentabilidade das famílias e à capacidade dos siste-
mas de saúde de darem suporte à população idosa. À medida que a tecnologia dos sensores
wireless evolui, dispositivos de baixo consumo, reduzida largura de banda e capacidade de ar-
mazenamento médio, surgem no mercado, com custos de aquisição bastante reduzidos. A mo-
nitorização de ambientes domésticos baseada em sensores wireless, fornece um meio seguro
e contido para pessoas idosas, permitindo que estas possam viver nas suas casas o máximo
tempo possível. Este trabalho introduz o Elder Monitoring System (EMoS), um sistema desen-
volvido no Mixed Simulator (MiXiM), onde foi implementado um protocolo de encaminhamento
Ad hoc On-demand Vector Routing (AODV) e um sistema de localização baseado no HORUS,
com a finalidade de monitorizar, num ambiente doméstico, pessoas idosas ou com necessidades
especiais. Os resultados obtidos desta investigação demonstram a viabilidade de construir um
sistema simulado para monitorização de pessoas num ambiente doméstico, onde aspectos de
hardware comercialmente disponível foram também discutidos.

Autor: Márcio Luís M. V. Nóbrega
Orientador: Prof. Doutor Renato Jorge Caleira Nunes
Co-Orientador: Prof. Doutor António Manuel Raminhos Cordeiro Grilo

Requisitos (Instalação em Linux 12.4 LTS)

> INSTALAÇÃO DO OMNET++

- OMNeT++ 4.2.2 (http://www.omnetpp.org/)
1- fazer download do ficheiro omnetpp-4.2.2-src.tgz - http://www.omnetpp.org/omnetpp/doc_details/2245-omnet-422-source--ide-tgz

2- abrir um terminal e instalar os pacotes necessários no ubuntu executando os seguintes comandos:
- sudo apt-get update
- sudo apt-get install build-essential gcc g++ bison flex perl \
tcl-dev tk-dev blt libxml2-dev zlib1g-dev openjdk-6-jre \
doxygen graphviz openmpi-bin libopenmpi-dev libpcap-dev

3- descomprimir na em /home 
- tar xvfz omnetpp-4.2.2-src.tgz

4- incluir no ficheiro ~/.bashrc as linha e reinciar a linha de comandos

export PATH=$PATH:$HOME/omnetpp-4.2.2/bin
export TCL_LIBRARY=/usr/share/tcltk/tcl8.5

5- ir para a pasta "omnetpp-4.2.2" e executar o comando:

./configure

6- verificar se o OMNeT++ está correctamente instalado executando os seguintes comandos:

- cd samples/dyna
- ./dyna

7 - instalar os icons de desktop e menu

- make install-menu-item
- make install-desktop-icon

8 - iniciar o IDE executando o seguinte comando

- omnetpp


> INSTALAÇÃO DO MIXIM/VEINS/EMoS

O EMoS foi desenvolvido dentro de uma versão modificada do MiXiM chamada VEINS que contém a simulação de obstáculos.

Para importar o VEINS para o IDE executar os seguintes passos:

- Seleccionar a opção File->Import
- Escolher o tipo de importação General->Existing Projects into Workspace
- Escolher a importação através de ficheiro comprimido e procurar o ficheiro "MiXiM-EMoS"

Após uma importação feita com sucesso seleccionar o projecto e executar a operação Run->BuildAll


> EXECUÇÃO DO EMoS

Para executar o EMoS efectuar os seguintes comandos no terminal:

- cd workspace/mixim/examples/EMoS
- editar os ficheiros omnetpp.tests.ini e o ficheiro omnetpp.tracking.ini para que os parâmetros de configuração seguintes estejam numa localização correcta:

ned-path=/home/mnobrega/workspace/mixim/src;/home/mnobrega/workspace/mixim
tkenv-image-path=/home/mnobrega/workspace/mixim/images/EMoS
load-libs=/home/mnobrega/workspace/mixim/src/modules/miximmodules /home/mnobrega/workspace/mixim/src/base/miximbase

- executar a simulação de localização do EMoS efectuando o seguinte comando:

./EMoS -f omnetpp.tracking.ini -u Tkenv -c online

Descrição das pastas que constituiem dentro do MiXiM o EMoS:

-> examples/EMoS : configuração geral através de ficheiros *.ini, ficheiros XML de configuração, resultados da simulação
-> src/modules/netw/EMoS : módulo network do AODV desenvolvido para o EMoS
-> src/modules/application/EMoS : implementação das diferentes camadas de aplicação existentes no sistema EMoS (Nó móvel, Nó estático e Nó base)
-> src/modules/messages/EMoS : mensagens usadas pela camada de aplicação e network
-> src/modules/node/EMoS : configuração física dos nós de sistema
-> src/base/utils/EMoS : informação de controlo enviada da camada Network para a camada Application









