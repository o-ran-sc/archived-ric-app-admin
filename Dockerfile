ARG SCHEMA_PATH=schemas
ARG STAGE_DIR=/tmp/ac-xapp

#==================================================================================
FROM nexus3.o-ran-sc.org:10004/bldr-ubuntu16-c-go:1-u16.04-nng1.1.1 as ricbuild

# to override repo base, pass in repo argument when running docker build:
# docker build --build-arg REPOBASE=http://abc.def.org . ....
ARG REPOBASE=https://gerrit.oran-osc.org/r
ARG SCHEMA_FILE
ARG SCHEMA_PATH
ARG STAGE_DIR

# Install necessary packages
WORKDIR ${STAGE_DIR}
RUN apt-get update  \
     && apt-get install -y \
     libcurl4-openssl-dev \
     libcurl3 \
     cmake \
     git \
     build-essential \
     automake \
     autoconf-archive \
     autoconf \
     pkg-config \
     gawk \
     libtool \
     wget \
     zlib1g-dev \
     libffi-dev \
     && apt-get clean

# Install mdclog using debian package hosted at packagecloud.io
ARG MDC_VER=0.0.3-1
RUN wget -nv --content-disposition https://packagecloud.io/o-ran-sc/master/packages/debian/stretch/mdclog_${MDC_VER}_amd64.deb/download.deb
RUN wget -nv --content-disposition https://packagecloud.io/o-ran-sc/master/packages/debian/stretch/mdclog-dev_${MDC_VER}_amd64.deb/download.deb
RUN dpkg -i mdclog_${MDC_VER}_amd64.deb
RUN dpkg -i mdclog-dev_${MDC_VER}_amd64.deb

# Install RMr using debian package hosted at packagecloud.io
ARG RMR_VER=1.3.0
RUN wget -nv --content-disposition https://packagecloud.io/o-ran-sc/staging/packages/debian/stretch/rmr_${RMR_VER}_amd64.deb/download.deb
RUN wget -nv --content-disposition https://packagecloud.io/o-ran-sc/staging/packages/debian/stretch/rmr-dev_${RMR_VER}_amd64.deb/download.deb
RUN dpkg -i rmr_${RMR_VER}_amd64.deb
RUN dpkg -i rmr-dev_${RMR_VER}_amd64.deb


## Install rapidjson
    #git checkout tags/v1.1.0 && \
RUN git clone https://github.com/Tencent/rapidjson && \
    cd rapidjson && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make install && \
    cd ${STAGE_DIR} && \
    rm -rf rapidjson


##-----------------------------------
# Now install the program
#------------------------------------
COPY ./ ${STAGE_DIR}
RUN export CPATH=$CPATH:/usr/local/include && \ 
    cd src && \
    make clean && \
    make install 
 
COPY ${SCHEMA_PATH}/* /etc/xapp/ 
COPY init/init_script.py /etc/xapp/init_script.py

#---------------------------------------------
# Build the final version
#FROM nexus3.o-ran-sc.org:10004/bldr-ubuntu16-c-go:1-u16.04-nng1.1.1

FROM ubuntu:16.04

ARG SCHEMA_PATH
ARG STAGE_DIR

# copy just the needed libraries install it into the final image
COPY --from=ricbuild ${STAGE_DIR}/*.deb /tmp/
COPY --from=ricbuild /usr/local/lib/libnng* /usr/local/lib/
RUN dpkg -i /tmp/*.deb
RUN apt-get update && \
    apt-get install -y libcurl3 python3 && \
    apt-get clean
COPY --from=ricbuild /etc/xapp/* /etc/xapp/
COPY --from=ricbuild /usr/local/bin/adm-ctrl-xapp /usr/local/bin/adm-ctrl-xapp
#COPY --from=ricbuild /usr/local/bin/e2e-test-client /usr/local/bin/e2e-test-client
#COPY --from=ricbuild /usr/local/bin/mock-e2term-server /usr/local/bin/mock-e2term-server
#COPY --from=ricbuild /usr/local/bin/e2e-perf-client /usr/local/bin/e2e-perf-client
#COPY --from=ricbuild /usr/local/bin/e2e-perf-server /usr/local/bin/e2e-perf-server
#COPY --from=ricbuild /usr/local/bin/mock-a1-server /usr/local/bin/mock-a1-server


RUN ldconfig


#ENV  PYTHONHOME=/opt/python3 \
#     PYTHONPATH=/opt/python3 \
ENV  RMR_RTG_SVC="127.0.0.1" \
     NAME=ADM_CTRL_XAPP \
     PORT=tcp:4560 \
     THREADS=1\
     VERBOSE=0 \
     MESSAGE_TYPE=10002 \
     RATE=1 \
     CONFIG_FILE=/opt/ric/config/config-file.json
     
      

CMD python3 /etc/xapp/init_script.py $CONFIG_FILE
