;; Copyright (C) 2025  gemmaro
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

(use-modules (guix packages)
             (guix download)
             (guix profiles)
             (gnu packages gettext)
             (gnu packages gdb)
             (gnu packages commencement))

(define-public gnu-gettext-0.26
  (package/inherit gnu-gettext
    (version "0.26")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://gnu/gettext/gettext-" version
                                  ".tar.gz"))
              (sha256 (base32
                       "0zqxpdc6xxf7rz69xfxiv7z32djvrsm64m80c05i36qy6yqg9b1r"))))))

(packages->manifest (list gnu-gettext-0.26 gcc-toolchain gdb))
