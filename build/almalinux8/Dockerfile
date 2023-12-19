FROM almalinux:8 AS build-stage

WORKDIR /usr/local/src
COPY slurm-*.tar.bz2 /usr/local/src/
COPY slurm.spec /usr/local/src/
COPY bridge-*.tar.gz /usr/local/src/
COPY bridge.spec /usr/local/src/

# Installs epel
# RUN dnf install -y epel-release

# Installs basic tools to be able to compile a bit
RUN dnf check-update && dnf update -y ; \
    dnf install -y git cmake gcc make rpm-build libtool ; \
    dnf install -y yum-utils;

# Enables Code Ready Builder repository to get munge-devel package
RUN dnf config-manager --set-enabled powertools

# Needed to build slurm
RUN dnf install -y munge munge-libs munge-devel mariadb-devel pam-devel readline-devel perl-ExtUtils-MakeMaker python36

# Needed to build bridge
RUN dnf install -y autoconf automake libtirpc-devel;

# Building slurm's rpm and installing them
RUN mkdir -p /root/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
RUN cp /usr/local/src/slurm-*.tar.bz2 /root/rpmbuild/SOURCES/
RUN cp /usr/local/src/slurm.spec /root/rpmbuild/SPECS/
RUN cd /root/rpmbuild && rpmbuild -v -ba SPECS/slurm.spec
RUN dnf -y localinstall /root/rpmbuild/RPMS/x86_64/*.rpm

# Building bridge rpm
RUN cp /usr/local/src/bridge-*.tar.gz /root/rpmbuild/SOURCES/
RUN cp /usr/local/src/bridge.spec /root/rpmbuild/SPECS/
RUN cd /root/rpmbuild && rpmbuild -v -ba SPECS/bridge.spec

# Trying to install it
RUN dnf -y localinstall /root/rpmbuild/RPMS/x86_64/bridge*.rpm

FROM almalinux:8 AS export-stage
RUN mkdir /dist
COPY --from=build-stage /root/rpmbuild/RPMS/x86_64/bridge*.rpm /dist/
COPY --from=build-stage /root/rpmbuild/SRPMS/bridge*.rpm /dist/
RUN ls -ls /dist/
